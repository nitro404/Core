#include "Application/ComponentRegistry.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/ThreadUtilities.h"

#include <windows.h>

#include <map>
#include <memory>

typedef HRESULT(WINAPI * GET_THREAD_DESCRIPTION_FUNCTION_TYPE)(HANDLE threadHandle, PWSTR * threadDescription);
typedef HRESULT(WINAPI * SET_THREAD_DESCRIPTION_FUNCTION_TYPE)(HANDLE threadHandle, PCWSTR threadDescription);

using ThreadNameMap = std::map<std::thread::native_handle_type, std::string>;

static constexpr const char * KERNEL32_DLL_NAME = "KERNEL32.DLL";
static constexpr const char * GET_THREAD_DESCRIPTIPTION_FUNCTION_NAME = "GetThreadDescription";
static constexpr const char * SET_THREAD_DESCRIPTIPTION_FUNCTION_NAME = "SetThreadDescription";

static std::unique_ptr<ThreadNameMap> s_threadNames;

static inline DWORD getThreadID(std::thread & thread) {
	return GetThreadId(static_cast<HANDLE>(thread.native_handle()));
}

#pragma pack(push,8)

struct THREADNAME_INFO {
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // thread name
	DWORD dwThreadID; // thread identifier (-1 for caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
};

#pragma pack(pop)

static void setThreadNameFallback(std::thread & thread, const std::string & threadName) {
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName.c_str();
		info.dwThreadID = getThreadID(thread);
		info.dwFlags = 0;

#pragma warning(push)
#pragma warning(disable: 6320 6322)

		__try{
			RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR *>(&info));
		}

		__except (EXCEPTION_EXECUTE_HANDLER) { }

#pragma warning(pop)
}

static std::string getThreadNameFallback(std::thread & thread) {
	if(s_threadNames == nullptr) {
		return {};
	}

	ThreadNameMap::const_iterator threadNameIterator(std::find_if(s_threadNames->cbegin(), s_threadNames->cend(), [&thread](const std::pair<std::thread::native_handle_type, std::string> currentThreadHandleNamePair) {
		return currentThreadHandleNamePair.first == thread.native_handle();
	}));

	if(threadNameIterator == s_threadNames->cend()) {
		return {};
	}

	return threadNameIterator->second;
}

namespace Utilities {

	std::string getThreadName(std::thread & thread) {
		HMODULE kernelHandle = GetModuleHandle(KERNEL32_DLL_NAME);

		if(kernelHandle != 0) {
			GET_THREAD_DESCRIPTION_FUNCTION_TYPE getThreadDescriptionFunction = reinterpret_cast<GET_THREAD_DESCRIPTION_FUNCTION_TYPE>(GetProcAddress(kernelHandle, GET_THREAD_DESCRIPTIPTION_FUNCTION_NAME));

			if(getThreadDescriptionFunction != nullptr) {
				PWSTR threadNameData = nullptr;
				HRESULT result = getThreadDescriptionFunction(thread.native_handle(), &threadNameData);

				if(SUCCEEDED(result)) {
					std::wstring threadName(threadNameData);

					LocalFree(threadNameData);

					return Utilities::wideStringToString(threadName);
				}
			}
		}

		return getThreadNameFallback(thread);
	}

	void setThreadName(std::thread & thread, const std::string & threadName) {
		HMODULE kernelHandle = GetModuleHandle(KERNEL32_DLL_NAME);

		if(kernelHandle != 0) {
			SET_THREAD_DESCRIPTION_FUNCTION_TYPE setThreadDescriptionFunction = reinterpret_cast<SET_THREAD_DESCRIPTION_FUNCTION_TYPE>(GetProcAddress(kernelHandle, SET_THREAD_DESCRIPTIPTION_FUNCTION_NAME));

			if(setThreadDescriptionFunction != nullptr && SUCCEEDED(setThreadDescriptionFunction(thread.native_handle(), Utilities::stringToWideString(threadName).data()))) {
				return;
			}
		}

		if(s_threadNames == nullptr) {
			s_threadNames = std::make_unique<ThreadNameMap>();
			ComponentRegistry::getInstance().addComponent(&s_threadNames);
		}

		(*s_threadNames)[thread.native_handle()] = threadName;

		setThreadNameFallback(thread, threadName);
	}

}

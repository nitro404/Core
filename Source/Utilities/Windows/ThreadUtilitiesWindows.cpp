#include "utilities/ThreadUtilities.h"

#include <windows.h>

static DWORD getThreadID(std::thread & thread) {
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

namespace Utilities {

	void setThreadName(std::thread & thread, const std::string & threadName) {
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

}

#include "Utilities/ThreadUtilities.h"

#include "Application/ComponentRegistry.h"
#include "Platform/Linux/LinuxUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <string_view>

using ThreadNameMap = std::map<std::thread::native_handle_type, std::string>;

static constexpr bool ENABLE_LONG_THREAD_NAME_CACHE = true;
static constexpr size_t MAX_PTHREAD_NAME_LENGTH = 15;

static std::unique_ptr<ThreadNameMap> s_longThreadNames;

static std::string_view getErrorMessage(int errorNumber) {
	switch(errorNumber) {
		case EINVAL:
			return "Thread name is invalid.";
		case ENOENT:
			return "Thread is either not initialized, already exited, or invalid.";
		case EPERM:
			return "Calling thread does not have permission.";
		case ERANGE:
			return "Thread name is too long.";
		case ESRCH:
			return "Thread not found.";
	}

	return "Unknown error.";
}

static bool isSuccess(int result, std::string_view errorMessage) {
	if(result == 0) {
		return true;
	}

	if(!errorMessage.empty()) {
		spdlog::error("{}: {}", errorMessage, getErrorMessage(result));
	}

	return false;
}

namespace Utilities {

	std::string getThreadName(std::thread & thread) {
		if(s_longThreadNames != nullptr) {
			ThreadNameMap::const_iterator longThreadNameIterator(std::find_if(s_longThreadNames->cbegin(), s_longThreadNames->cend(), [&thread](const std::pair<std::thread::native_handle_type, std::string> currentThreadHandleNamePair) {
				return currentThreadHandleNamePair.first == thread.native_handle();
			}));

			if(longThreadNameIterator != s_longThreadNames->cend()) {
				return longThreadNameIterator->second;
			}
		}

		char threadName[MAX_PTHREAD_NAME_LENGTH + 1];

		if(!isSuccess(pthread_getname_np(thread.native_handle(), threadName, MAX_PTHREAD_NAME_LENGTH + 1), "Failed to get thread name")) {
			return "";
		}

		return threadName;
	}

	bool setThreadName(std::thread & thread, const std::string & threadName) {
		if(threadName.length() > MAX_PTHREAD_NAME_LENGTH) {
			spdlog::warn("New PThread name length of {} exceeds the limit of {}, excess characters will be ignored.", threadName.length(), MAX_PTHREAD_NAME_LENGTH);

			std::string truncatedThreadName(std::string_view(threadName.data(), MAX_PTHREAD_NAME_LENGTH));

			if(!isSuccess(pthread_setname_np(thread.native_handle(), truncatedThreadName.data()), fmt::format("Failed to set truncated thread name to '{}'", truncatedThreadName))) {
				return false;
			}

			if(ENABLE_LONG_THREAD_NAME_CACHE) {
				if(s_longThreadNames == nullptr) {
					s_longThreadNames = std::make_unique<ThreadNameMap>();
					ComponentRegistry::getInstance().addComponent(&s_longThreadNames);
				}

				(*s_longThreadNames)[thread.native_handle()] = threadName;
			}

			return true;
		}

		return isSuccess(pthread_setname_np(thread.native_handle(), threadName.data()), fmt::format("Failed to set thread name to '{}'", threadName));
	}

}

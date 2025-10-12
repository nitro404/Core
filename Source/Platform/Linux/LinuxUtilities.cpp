#include "LinuxUtilities.h"

#include <spdlog/spdlog.h>

#include <string.h>

std::string_view LinuxUtilities::getErrorMessage(int errorNumber) {
	return strerror(errorNumber);
}

bool LinuxUtilities::isSuccess(int result, std::string_view errorMessage) {
	if(result == 0) {
		return true;
	}

	if(!errorMessage.empty()) {
		spdlog::error("{}: {}", errorMessage, getErrorMessage(result));
	}

	return false;
}

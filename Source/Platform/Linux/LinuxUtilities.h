#ifndef _LINUX_UTILITIES_H_
#define _LINUX_UTILITIES_H_

#include <string>

namespace LinuxUtilities {

	std::string_view getErrorMessage(int errorNumber);
	bool isSuccess(int result, std::string_view errorMessage = {});

}

#endif // _LINUX_UTILITIES_H_

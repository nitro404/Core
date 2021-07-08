#ifndef _WINDOWS_UTILITIES_H_
#define _WINDOWS_UTILITIES_H_

#define _WIN32_DCOM

#include <any>
#include <comdef.h>
#include <map>
#include <string>
#include <vector>
#include <Wbemidl.h>

namespace WindowsUtilities {

	std::string getErrorMessage(HRESULT result);
	std::string getRegistryEntry(const std::string & key, const std::string & entryName, bool * error = nullptr);
	std::any variantToAny(VARIANT variant, bool * error = nullptr);
	std::vector<std::map<std::string, std::any>> getWindowsManagementInstrumentationEntries(const std::string & providerClassName, const std::vector<std::string> & propertyNames, bool * error = nullptr);
	std::any getWindowsManagementInstrumentationEntry(const std::string& providerClassName, const std::string& propertyName, bool* error = nullptr);

}

#endif // _WINDOWS_UTILITIES_H_

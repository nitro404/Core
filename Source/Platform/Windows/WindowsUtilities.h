#ifndef _WINDOWS_UTILITIES_H_
#define _WINDOWS_UTILITIES_H_

#define _WIN32_DCOM

#include <comdef.h>
#include <Wbemidl.h>

#include <any>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace WindowsUtilities {

	std::string getErrorMessage(HRESULT result);
	std::string getRegistryEntry(const std::string & key, const std::string & entryName, bool * error);
	std::optional<std::string> getRegistryEntry(const std::string & key, const std::string & entryName);
	std::any variantToAny(VARIANT variant, bool * error = nullptr);
	std::vector<std::map<std::string, std::any>> getWindowsManagementInstrumentationEntries(const std::string & providerClassName, const std::vector<std::string> & propertyNames, bool * error);
	std::optional<std::vector<std::map<std::string, std::any>>> getWindowsManagementInstrumentationEntries(const std::string & providerClassName, const std::vector<std::string> & propertyNames);
	std::any getWindowsManagementInstrumentationEntry(const std::string & providerClassName, const std::string & propertyName, bool * error = nullptr);

}

#endif // _WINDOWS_UTILITIES_H_

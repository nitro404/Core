#include "WindowsUtilities.h"

#include "Utilities/StringUtilities.h"

#include <stdio.h>

#pragma comment(lib, "wbemuuid.lib")

std::string WindowsUtilities::getErrorMessage(HRESULT result) {
	return _com_error(result).ErrorMessage();
}

std::string WindowsUtilities::getRegistryEntry(const std::string & key, const std::string & entryName, bool * error) {
	HKEY hkey;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hkey) != ERROR_SUCCESS) {
		printf("Failed to open registry key.\n");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	DWORD registryValueType;
	DWORD registryValueSize;

	if(RegQueryValueEx(hkey, entryName.c_str(), nullptr, &registryValueType, nullptr, &registryValueSize) != ERROR_SUCCESS) {
		RegCloseKey(hkey);

		printf("Failed to read registry entry value size.\n");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	if(registryValueType != REG_SZ) {
		RegCloseKey(hkey);

		printf("Invalid registry entry value type.\n");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	std::wstring value(registryValueSize / sizeof(wchar_t), L'\0');

	if(RegQueryValueEx(hkey, entryName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(&value[0]), &registryValueSize) != ERROR_SUCCESS) {
		RegCloseKey(hkey);

		printf("Failed to read registry entry value.\n");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	RegCloseKey(hkey);

	if(error != nullptr) {
		*error = false;
	}

	return Utilities::wideStringToString(value);
}

std::optional<std::string> WindowsUtilities::getRegistryEntry(const std::string & key, const std::string & entryName) {
	bool error = false;

	std::string registryEntry(WindowsUtilities::getRegistryEntry(key, entryName, &error));

	if(error) {
		return {};
	}

	return registryEntry;
}

std::any WindowsUtilities::variantToAny(VARIANT variant, bool * error) {
	std::any any;

	switch(variant.vt) {
		case VT_BOOL: {
			any = static_cast<bool>(variant.boolVal);
			break;
		}

		case VT_I1: {
			any = static_cast<int8_t>(variant.bVal);
			break;
		}

		case VT_UI1: {
			any = static_cast<uint8_t>(variant.bVal);
			break;
		}

		case VT_I2: {
			any = static_cast<int16_t>(variant.iVal);
			break;
		}

		case VT_UI2: {
			any = static_cast<uint16_t>(variant.uiVal);
			break;
		}

		case VT_INT:
		case VT_I4: {
			any = static_cast<int32_t>(variant.intVal);
			break;
		}

		case VT_UINT:
		case VT_UI4: {
			any = static_cast<uint32_t>(variant.uintVal);
			break;
		}

		case VT_I8: {
			any = static_cast<int64_t>(variant.llVal);
			break;
		}

		case VT_UI8: {
			any = static_cast<uint64_t>(variant.ullVal);
			break;
		}

		case VT_R4: {
			any = static_cast<float>(variant.fltVal);
			break;
		}

		case VT_R8: {
			any = static_cast<double>(variant.dblVal);
			break;
		}

		case VT_CY: {
			any = variant.cyVal;
			break;
		}

		case VT_DATE: {
			any = variant.date;
			break;
		}

		case VT_BSTR: {
			any = Utilities::wideStringToString(std::wstring(variant.bstrVal, SysStringLen(variant.bstrVal)));
			break;
		}

		case VT_DECIMAL: {
			any = *variant.pdecVal;
			break;
		}

		case VT_VARIANT:
		case VT_UNKNOWN:
		case VT_EMPTY:
		case VT_NULL:
		case VT_DISPATCH:
		case VT_ERROR:
		case VT_VOID:
		case VT_HRESULT:
		case VT_PTR:
		case VT_SAFEARRAY:
		case VT_CARRAY:
		case VT_USERDEFINED:
		case VT_LPSTR:
		case VT_LPWSTR:
		case VT_RECORD:
		case VT_INT_PTR:
		case VT_UINT_PTR:
		case VT_FILETIME:
		case VT_BLOB:
		case VT_STREAM:
		case VT_STORAGE:
		case VT_STREAMED_OBJECT:
		case VT_STORED_OBJECT:
		case VT_BLOB_OBJECT:
		case VT_CF:
		case VT_CLSID:
		case VT_VERSIONED_STREAM:
		case VT_BSTR_BLOB:
		case VT_VECTOR:
		case VT_ARRAY:
		case VT_BYREF:
		case VT_RESERVED:
		case VT_ILLEGAL: {
			if(error != nullptr) {
				*error = true;
			}

			return {};
		}
	}

	if(error != nullptr) {
		*error = false;
	}

	return any;
}

std::vector<std::map<std::string, std::any>> WindowsUtilities::getWindowsManagementInstrumentationEntries(const std::string & providerClassName, const std::vector<std::string> & propertyNames, bool * error) {
	HRESULT result;

	// Initialize COM
	result = CoInitializeEx(0, COINIT_MULTITHREADED);
	if(FAILED(result)) {
		printf("Failed to initialize COM library: %s\n", WindowsUtilities::getErrorMessage(result).c_str());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	static bool securityInitialized = false;

	if(!securityInitialized) {
		// Set general COM security levels
		result =  CoInitializeSecurity(
			nullptr,
			-1,                          // COM authentication
			nullptr,                     // Authentication services
			nullptr,                     // Reserved
			RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
			RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
			nullptr,                     // Authentication info
			EOAC_NONE,                   // Additional capabilities
			nullptr);                    // Reserved

		if(FAILED(result) && result != RPC_E_TOO_LATE)
		{
			printf("Failed to initialize security: %s\n", WindowsUtilities::getErrorMessage(result).c_str());

			CoUninitialize();

			if(error != nullptr) {
				*error = true;
			}

			return {};
		}

		securityInitialized = true;
	}

	// Obtain the initial locator to WMI
	IWbemLocator * locator = nullptr;

	result = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *) &locator);

	if(FAILED(result)) {
		printf("Failed to create IWbemLocator object: %s\n", WindowsUtilities::getErrorMessage(result).c_str());

		CoUninitialize();

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// Connect to WMI through the IWbemLocator::ConnectServer method
	IWbemServices * service = nullptr;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer service
	// to make IWbemServices calls.
	result = locator->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		nullptr,                 // User name. NULL = current user
		nullptr,                 // User password. NULL = current
		0,                       // Locale. NULL indicates current
		0,                       // Security flags
		0,                       // Authority (for example, Kerberos)
		nullptr,                 // Context object
		&service);               // Pointer to IWbemServices proxy

	if(FAILED(result)) {
		printf("Could not connect: %s\n", WindowsUtilities::getErrorMessage(result).c_str());

		locator->Release();
		CoUninitialize();

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// Set security levels on the proxy
	result = CoSetProxyBlanket(
		service,                     // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		nullptr,                     // Server principal name
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		nullptr,                     // Client identity
		EOAC_NONE);                  // Proxy capabilities

	if(FAILED(result)) {
		printf("Could not set proxy blanket: %s\n", WindowsUtilities::getErrorMessage(result).c_str());

		service->Release();
		locator->Release();
		CoUninitialize();

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// Use the IWbemServices pointer to make requests of WMI
	std::string query("SELECT * FROM " + providerClassName);
	IEnumWbemClassObject * enumerator = nullptr;
	result = service->ExecQuery(
		bstr_t("WQL"),
		bstr_t(query.c_str()),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		nullptr,
		&enumerator);

	if(FAILED(result)) {
		printf("Query for '%s' failed: %s\n", providerClassName.c_str(), WindowsUtilities::getErrorMessage(result).c_str());

		service->Release();
		locator->Release();
		CoUninitialize();

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// Get the data from the query result
	IWbemClassObject * classObject = nullptr;
	ULONG returnValue = 0;
	std::vector<std::map<std::string, std::any>> results;

	while(enumerator) {
		result = enumerator->Next(WBEM_INFINITE, 1, &classObject, &returnValue);

		if(returnValue == 0) {
			break;
		}

		VARIANT property = { };
		std::wstring formattedPropertyName;
		std::map<std::string, std::any> data;

		for(const std::string & propertyName : propertyNames) {
			// Get the value of the requested property
			formattedPropertyName = std::wstring(propertyName.begin(), propertyName.end());
			result = classObject->Get(formattedPropertyName.c_str(), 0, &property, 0, 0);

			if(SUCCEEDED(result)) {
				data[propertyName] = WindowsUtilities::variantToAny(property);
			}

			VariantClear(&property);
		}

		results.emplace_back(data);

		classObject->Release();
	}

	// Cleanup
	service->Release();
	locator->Release();

	if(enumerator != nullptr) {
		enumerator->Release();
	}

	CoUninitialize();

	if(error != nullptr) {
		*error = false;
	}

	return results;
}

std::optional<std::vector<std::map<std::string, std::any>>> WindowsUtilities::getWindowsManagementInstrumentationEntries(const std::string & providerClassName, const std::vector<std::string> & propertyNames) {
	bool error = false;

	std::vector<std::map<std::string, std::any>> windowsManagementInstrumentationEntries(WindowsUtilities::getWindowsManagementInstrumentationEntries(providerClassName, propertyNames, &error));

	if(error) {
		return {};
	}

	return windowsManagementInstrumentationEntries;
}

std::any WindowsUtilities::getWindowsManagementInstrumentationEntry(const std::string & providerClassName, const std::string & propertyName, bool * error) {
	std::vector<std::map<std::string, std::any>> results = WindowsUtilities::getWindowsManagementInstrumentationEntries(providerClassName, std::vector { propertyName }, error);

	if (results.empty()) {
		return {};
	}

	return results[0][propertyName];
}

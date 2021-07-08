#include "DeviceInformationBridgeWindows.h"

#include "WindowsUtilities.h"
#include "Utilities/StringUtilities.h"

static const std::string COMPUTER_SYSTEM_PROVIDER_CLASS_NAME("Win32_ComputerSystem");
static const std::string COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME("Win32_ComputerSystemProduct");
static const std::string OPERATING_SYSTEM_PROVIDER_CLASS_NAME("Win32_OperatingSystem");
static const std::string SYSTEM_ENCLOSURE_PROVIDER_CLASS_NAME("Win32_SystemEnclosure");
static const std::string NETWORK_ADAPTER_PROVIDER_CLASS_NAME("Win32_NetworkAdapter");

DeviceInformationBridgeWindows::DeviceInformationBridgeWindows() = default;

DeviceInformationBridgeWindows::~DeviceInformationBridgeWindows() = default;

std::string DeviceInformationBridgeWindows::getHostName() {
	static std::string hostName;

	if(hostName.empty()) {
		hostName = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PROVIDER_CLASS_NAME, "Name"));
	}

	return hostName;
}

std::string DeviceInformationBridgeWindows::getModel() {
	static std::string model;
	static const std::string INVALID_MODEL("System Product Name");

	if(model.empty()) {
		model = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME, "Name"));

		if(model == INVALID_MODEL) {
			model = UNKNOWN_VALUE;
		}
	}

	return model;
}

std::string DeviceInformationBridgeWindows::getModelIdentifier() {
	static std::string modelIdentifier;
	static const std::string INVALID_MODEL_IDENTIFIER("System Serial Number");

	if(modelIdentifier.empty()) {
		modelIdentifier = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME, "IdentifyingNumber"));

		if(modelIdentifier == INVALID_MODEL_IDENTIFIER) {
			modelIdentifier = UNKNOWN_VALUE;
		}
	}

	return modelIdentifier;
}

std::string DeviceInformationBridgeWindows::getManufacturerName() {
	static std::string manufacturerName;
	static const std::string INVALID_MANUFACTURER_NAME("System manufacturer");

	if(manufacturerName.empty()) {
		manufacturerName = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME, "Vendor"));

		if(manufacturerName == INVALID_MANUFACTURER_NAME) {
			manufacturerName = UNKNOWN_VALUE;
		}
	}

	return manufacturerName;
}

std::string DeviceInformationBridgeWindows::getUniqueIdentifier() {
	static std::string uniqueIdentifier;
	static const std::string INVALID_UUID("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF");

	if(uniqueIdentifier.empty()) {
		uniqueIdentifier = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME, "UUID"));

		if(uniqueIdentifier.empty() || uniqueIdentifier == INVALID_UUID) {
			uniqueIdentifier = getMACAddress();

			if(uniqueIdentifier.empty()) {
				uniqueIdentifier = std::any_cast<std::string>(WindowsUtilities::getRegistryEntry("SOFTWARE\\Microsoft\\Cryptography", "MachineGuid"));
			}
		}
	}

	return uniqueIdentifier;
}

std::string DeviceInformationBridgeWindows::getOperatingSystemName() {
	static std::string operatingSystemName;

	if(operatingSystemName.empty()) {
		operatingSystemName = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(OPERATING_SYSTEM_PROVIDER_CLASS_NAME, "Name"));
		size_t separatorIndex = operatingSystemName.find_first_of("|");

		if(separatorIndex != std::string::npos) {
			operatingSystemName = Utilities::trimString(operatingSystemName.substr(0, separatorIndex));
		}
	}

	return operatingSystemName;
}

std::string DeviceInformationBridgeWindows::getOperatingSystemArchitecture() {
	static std::string operatingSystemArchitecture;

	if(operatingSystemArchitecture.empty()) {
		operatingSystemArchitecture = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(OPERATING_SYSTEM_PROVIDER_CLASS_NAME, "OSArchitecture"));
	}

	return operatingSystemArchitecture;
}

std::string DeviceInformationBridgeWindows::getOperatingSystemVersion() {
	static std::string operatingSystemVersion;

	if(operatingSystemVersion.empty()) {
		operatingSystemVersion = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(OPERATING_SYSTEM_PROVIDER_CLASS_NAME, "Version"));
	}

	return operatingSystemVersion;
}

std::string DeviceInformationBridgeWindows::getMACAddress(NetworkConnectionType connectionType) {
	static std::string macAddress;

	if(macAddress.empty()) {
		std::vector<DeviceInformationBridge::NetworkAdapterInformation> networkAdapters = getNetworkAdapterInformation();

		for(const DeviceInformationBridge::NetworkAdapterInformation & networkAdapter : networkAdapters) {
			if(networkAdapter.type == connectionType) {
				return networkAdapter.macAddress;
			}
		}
	}

	return macAddress;
}

std::vector<DeviceInformationBridge::NetworkAdapterInformation> DeviceInformationBridgeWindows::getNetworkAdapterInformation() {
	static const std::string ADAPTER_DEVICE_ID_PROPERTY_NAME("DeviceID");
	static const std::string ADAPTER_MAC_ADDRESS_PROPERTY_NAME("MACAddress");
	static const std::string ADAPTER_MANUFACTURER_PROPERTY_NAME("Manufacturer");
	static const std::string ADAPTER_NAME_PROPERTY_NAME("Name");
	static const std::string ADAPTER_NET_CONNECTION_ID_PROPERTY_NAME("NetConnectionID");
	static const std::string ADAPTER_SERVICE_NAME_PROPERTY_NAME("ServiceName");
	static const std::string ADAPTER_PHYSICAL_ADAPTER_PROPERTY_NAME("PhysicalAdapter");
	static const std::string INVALID_MANUFACTURER_VALUE("Microsoft");
	static const std::string ETHERNET_NET_CONNECTION_ID_VALUE("Local Area Connection");
	static const std::string WIRELESS_NET_CONNECTION_ID_VALUE("Wireless Network Connection");
	static const std::vector<std::string> networkAdapterPropertyNames = {
		ADAPTER_DEVICE_ID_PROPERTY_NAME,
		ADAPTER_MAC_ADDRESS_PROPERTY_NAME,
		ADAPTER_MANUFACTURER_PROPERTY_NAME,
		ADAPTER_NAME_PROPERTY_NAME,
		ADAPTER_NET_CONNECTION_ID_PROPERTY_NAME,
		ADAPTER_SERVICE_NAME_PROPERTY_NAME,
		ADAPTER_PHYSICAL_ADAPTER_PROPERTY_NAME
	};

	std::vector<DeviceInformationBridge::NetworkAdapterInformation> networkAdapterInfoCollection;
	std::vector<std::map<std::string, std::any>> networkData = WindowsUtilities::getWindowsManagementInstrumentationEntries(NETWORK_ADAPTER_PROVIDER_CLASS_NAME, networkAdapterPropertyNames);

	for(std::vector<std::map<std::string, std::any>>::const_iterator i = networkData.begin(); i != networkData.end(); ++i) {
		const std::map<std::string, std::any> networkAdapterData = *i;

		std::map<std::string, std::any>::const_iterator physicalAdapter(networkAdapterData.find(ADAPTER_PHYSICAL_ADAPTER_PROPERTY_NAME));

		if(physicalAdapter == networkAdapterData.end() || !physicalAdapter->second.has_value() || !std::any_cast<bool>(networkAdapterData.at(ADAPTER_PHYSICAL_ADAPTER_PROPERTY_NAME))) {
			continue;
		}

		std::map<std::string, std::any>::const_iterator manufacturer(networkAdapterData.find(ADAPTER_MANUFACTURER_PROPERTY_NAME));

		if(manufacturer == networkAdapterData.end() || !manufacturer->second.has_value() || std::any_cast<std::string>(manufacturer->second) == INVALID_MANUFACTURER_VALUE) {
			continue;
		}

		NetworkAdapterInformation networkAdapterInfo = {};

		std::map<std::string, std::any>::const_iterator netConnectionID(networkAdapterData.find(ADAPTER_NET_CONNECTION_ID_PROPERTY_NAME));

		if(netConnectionID != networkAdapterData.end() && netConnectionID->second.has_value()) {
			std::string netConnectionIDValue = std::any_cast<std::string>(netConnectionID->second);

			if(netConnectionIDValue == ETHERNET_NET_CONNECTION_ID_VALUE) {
				networkAdapterInfo.type = DeviceInformationBridge::NetworkConnectionType::Wired;
			}
			else if(netConnectionIDValue == WIRELESS_NET_CONNECTION_ID_VALUE) {
				networkAdapterInfo.type = DeviceInformationBridge::NetworkConnectionType::Wireless;
			}
			else {
				continue;
			}
		}
		else {
			continue;
		}

		std::map<std::string, std::any>::const_iterator macAddress(networkAdapterData.find(ADAPTER_MAC_ADDRESS_PROPERTY_NAME));

		if(macAddress != networkAdapterData.end() && macAddress->second.has_value()) {
			networkAdapterInfo.macAddress = std::any_cast<std::string>(macAddress->second);
		}
		else {
			continue;
		}

		std::map<std::string, std::any>::const_iterator deviceID(networkAdapterData.find(ADAPTER_DEVICE_ID_PROPERTY_NAME));

		if(deviceID != networkAdapterData.end() && deviceID->second.has_value()) {
			networkAdapterInfo.id = std::any_cast<std::string>(deviceID->second);
		}

		std::map<std::string, std::any>::const_iterator name(networkAdapterData.find(ADAPTER_NAME_PROPERTY_NAME));

		if(name != networkAdapterData.end() && name->second.has_value()) {
			networkAdapterInfo.name = std::any_cast<std::string>(name->second);
		}

		networkAdapterInfo.manufacturer = std::any_cast<std::string>(manufacturer->second);

		std::map<std::string, std::any>::const_iterator serviceName(networkAdapterData.find(ADAPTER_SERVICE_NAME_PROPERTY_NAME));

		if(serviceName != networkAdapterData.end() && serviceName->second.has_value()) {
			networkAdapterInfo.serviceName = std::any_cast<std::string>(serviceName->second);
		}

		networkAdapterInfoCollection.emplace_back(networkAdapterInfo);
	}

	return networkAdapterInfoCollection;
}

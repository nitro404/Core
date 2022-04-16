#include "DeviceInformationBridgeWindows.h"

#include "WindowsUtilities.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <powerbase.h>

#pragma comment(lib, "Powrprof.lib")

static const std::string COMPUTER_SYSTEM_PROVIDER_CLASS_NAME("Win32_ComputerSystem");
static const std::string COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME("Win32_ComputerSystemProduct");
static const std::string OPERATING_SYSTEM_PROVIDER_CLASS_NAME("Win32_OperatingSystem");
static const std::string SYSTEM_ENCLOSURE_PROVIDER_CLASS_NAME("Win32_SystemEnclosure");
static const std::string NETWORK_ADAPTER_PROVIDER_CLASS_NAME("Win32_NetworkAdapter");
static const std::string BASE_BOARD_PROVIDER_CLASS_NAME("Win32_BaseBoard");
static const std::string PROCESSOR_PROVIDER_CLASS_NAME("Win32_Processor");

DeviceInformationBridgeWindows::DeviceInformationBridgeWindows() { }

DeviceInformationBridgeWindows::~DeviceInformationBridgeWindows() { }

std::string DeviceInformationBridgeWindows::getHostName() {
	static std::string s_hostName;

	if(s_hostName.empty()) {
		s_hostName = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PROVIDER_CLASS_NAME, "Name"));
	}

	return s_hostName;
}

std::string DeviceInformationBridgeWindows::getDeviceModel() {
	static std::optional<std::string> s_optionalModel;
	static const std::string INVALID_MODEL("System Product Name");

	if(!s_optionalModel.has_value()) {
		s_optionalModel = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME, "Name"));

		if(s_optionalModel == INVALID_MODEL) {
			s_optionalModel = Utilities::emptyString;
		}
	}

	return s_optionalModel.value();
}

std::string DeviceInformationBridgeWindows::getDeviceModelIdentifier() {
	static std::optional<std::string> s_optionalModelIdentifier;
	static const std::string INVALID_MODEL_IDENTIFIER("System Serial Number");

	if(!s_optionalModelIdentifier.has_value()) {
		s_optionalModelIdentifier = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME, "IdentifyingNumber"));

		if(s_optionalModelIdentifier.value() == INVALID_MODEL_IDENTIFIER) {
			s_optionalModelIdentifier = Utilities::emptyString;
		}
	}

	return s_optionalModelIdentifier.value();
}

std::string DeviceInformationBridgeWindows::getDeviceType() {
	static std::optional<std::string> s_optionalType;

	if(!s_optionalType.has_value()) {
		SYSTEM_POWER_CAPABILITIES systemPowerCapabilities;

		if(CallNtPowerInformation(SystemPowerCapabilities, nullptr, 0, &systemPowerCapabilities, sizeof(SYSTEM_POWER_CAPABILITIES)) == 0) {
			s_optionalType = systemPowerCapabilities.LidPresent ? "Laptop" : "Desktop";
		}
	}

	return s_optionalType.value();
}

std::string DeviceInformationBridgeWindows::getDeviceManufacturerName() {
	static std::optional<std::string> s_optionalManufacturerName;
	static const std::string INVALID_MANUFACTURER_NAME("System manufacturer");

	if(!s_optionalManufacturerName.has_value()) {
		s_optionalManufacturerName = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME, "Vendor"));

		if(s_optionalManufacturerName.value() == INVALID_MANUFACTURER_NAME) {
			s_optionalManufacturerName = Utilities::emptyString;
		}
	}

	return s_optionalManufacturerName.value();
}

std::string DeviceInformationBridgeWindows::getDeviceUniqueIdentifier() {
	static std::string s_uniqueIdentifier;
	static const std::string INVALID_UUID("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF");

	if(s_uniqueIdentifier.empty()) {
		s_uniqueIdentifier = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME, "UUID"));

		if(s_uniqueIdentifier.empty() || s_uniqueIdentifier == INVALID_UUID) {
			s_uniqueIdentifier = getMACAddress();

			if(s_uniqueIdentifier.empty()) {
				s_uniqueIdentifier = std::any_cast<std::string>(WindowsUtilities::getRegistryEntry("SOFTWARE\\Microsoft\\Cryptography", "MachineGuid"));
			}
		}
	}

	return s_uniqueIdentifier;
}

std::string DeviceInformationBridgeWindows::getOperatingSystemName() {
	static std::string s_operatingSystemName;

	if(s_operatingSystemName.empty()) {
		s_operatingSystemName = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(OPERATING_SYSTEM_PROVIDER_CLASS_NAME, "Name"));
		size_t separatorIndex = s_operatingSystemName.find_first_of("|");

		if(separatorIndex != std::string::npos) {
			s_operatingSystemName = Utilities::trimString(s_operatingSystemName.substr(0, separatorIndex));
		}
	}

	return s_operatingSystemName;
}

std::string DeviceInformationBridgeWindows::getOperatingSystemVersion() {
	static std::string s_operatingSystemVersion;

	if(s_operatingSystemVersion.empty()) {
		s_operatingSystemVersion = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(OPERATING_SYSTEM_PROVIDER_CLASS_NAME, "Version"));
	}

	return s_operatingSystemVersion;
}

std::string DeviceInformationBridgeWindows::getOperatingSystemArchitecture() {
	static std::string s_operatingSystemArchitecture;

	if(s_operatingSystemArchitecture.empty()) {
		s_operatingSystemArchitecture = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(OPERATING_SYSTEM_PROVIDER_CLASS_NAME, "OSArchitecture"));
	}

	return s_operatingSystemArchitecture;
}

std::string DeviceInformationBridgeWindows::getProcessorName() {
	static std::string s_processorName;

	if(s_processorName.empty()) {
		s_processorName = std::any_cast<std::string>(WindowsUtilities::getWindowsManagementInstrumentationEntry(PROCESSOR_PROVIDER_CLASS_NAME, "Name"));
	}

	return s_processorName;
}

std::string DeviceInformationBridgeWindows::getMotherboardName() {
	static const std::string MANUFACTURER_PROPERTY_NAME("Manufacturer");
	static const std::string PRODUCT_PROPERTY_NAME("Product");
	static const std::string VERSION_PROPERTY_NAME("Version");
	static const std::vector<std::string> BASE_BOARD_PROPERTY_NAMES = {
		MANUFACTURER_PROPERTY_NAME,
		PRODUCT_PROPERTY_NAME,
		VERSION_PROPERTY_NAME
	};

	static std::string s_motherboardName;

	if(s_motherboardName.empty()) {
		std::optional<std::vector<std::map<std::string, std::any>>> optionalMotherboardInfo(WindowsUtilities::getWindowsManagementInstrumentationEntries(BASE_BOARD_PROVIDER_CLASS_NAME, BASE_BOARD_PROPERTY_NAMES));

		if(optionalMotherboardInfo.has_value() && !optionalMotherboardInfo->empty()) {
			const std::map<std::string, std::any> & motherboardInfo(optionalMotherboardInfo.value().at(0));

			s_motherboardName = fmt::format("{} {} {}",
				std::any_cast<std::string>(motherboardInfo.at(MANUFACTURER_PROPERTY_NAME)),
				std::any_cast<std::string>(motherboardInfo.at(PRODUCT_PROPERTY_NAME)),
				std::any_cast<std::string>(motherboardInfo.at(VERSION_PROPERTY_NAME)));
		}
	}

	return s_motherboardName;
}

std::string DeviceInformationBridgeWindows::getMACAddress(NetworkConnectionType connectionType) {
	static std::string s_macAddress;

	if(s_macAddress.empty()) {
		std::vector<DeviceInformationBridge::NetworkAdapterInformation> networkAdapters = getNetworkAdapterInformation();

		for(const DeviceInformationBridge::NetworkAdapterInformation & networkAdapter : networkAdapters) {
			if(networkAdapter.type == connectionType) {
				s_macAddress = networkAdapter.macAddress;
			}
		}
	}

	return s_macAddress;
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

	std::optional<std::vector<std::map<std::string, std::any>>> networkData = WindowsUtilities::getWindowsManagementInstrumentationEntries(NETWORK_ADAPTER_PROVIDER_CLASS_NAME, networkAdapterPropertyNames);

	if(!networkData.has_value()) {
		return {};
	}

	std::vector<DeviceInformationBridge::NetworkAdapterInformation> networkAdapterInfoCollection;

	for(std::vector<std::map<std::string, std::any>>::const_iterator i = networkData->begin(); i != networkData->end(); ++i) {
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

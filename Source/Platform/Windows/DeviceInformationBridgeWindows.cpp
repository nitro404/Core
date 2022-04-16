#include "DeviceInformationBridgeWindows.h"

#include "WindowsUtilities.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <netlistmgr.h>
#include <powerbase.h>
#include <sysinfoapi.h>
#include <timezoneapi.h>
#include <winnls.h>
#include <wtypes.h>

#pragma comment(lib, "Powrprof.lib")

static const std::string COMPUTER_SYSTEM_PROVIDER_CLASS_NAME("Win32_ComputerSystem");
static const std::string COMPUTER_SYSTEM_PRODUCT_PROVIDER_CLASS_NAME("Win32_ComputerSystemProduct");
static const std::string OPERATING_SYSTEM_PROVIDER_CLASS_NAME("Win32_OperatingSystem");
static const std::string SYSTEM_ENCLOSURE_PROVIDER_CLASS_NAME("Win32_SystemEnclosure");
static const std::string NETWORK_ADAPTER_PROVIDER_CLASS_NAME("Win32_NetworkAdapter");
static const std::string BASE_BOARD_PROVIDER_CLASS_NAME("Win32_BaseBoard");
static const std::string PROCESSOR_PROVIDER_CLASS_NAME("Win32_Processor");
static const std::string VIDEO_CONTROLLER_PROVIDER_CLASS_NAME("Win32_VideoController");
static const std::string PHYSICAL_MEMORY_PROVIDER_CLASS_NAME("Win32_PhysicalMemory");

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

std::vector<std::string> DeviceInformationBridgeWindows::getGraphicsCardNames() {
	static const std::string NAME_PROPERTY_NAME("Name");
	static const std::vector<std::string> VIDEO_CONTROLLER_PROPERTY_NAMES = {
		NAME_PROPERTY_NAME
	};

	static std::optional<std::vector<std::string>> s_graphicsCardNames;

	if(!s_graphicsCardNames.has_value()) {
		std::vector<std::string> graphicsCardNames;
		std::optional<std::vector<std::map<std::string, std::any>>> optionalGraphicsCardInfo(WindowsUtilities::getWindowsManagementInstrumentationEntries(VIDEO_CONTROLLER_PROVIDER_CLASS_NAME, VIDEO_CONTROLLER_PROPERTY_NAMES));

		if(optionalGraphicsCardInfo.has_value()) {
			for(std::vector<std::map<std::string, std::any>>::const_iterator i = optionalGraphicsCardInfo->cbegin(); i != optionalGraphicsCardInfo->cend(); ++i) {
				graphicsCardNames.emplace_back(std::any_cast<std::string>(i->at(NAME_PROPERTY_NAME)));
			}
		}

		s_graphicsCardNames = std::move(graphicsCardNames);
	}

	return s_graphicsCardNames.value();
}

DeviceInformationBridge::MemoryStatus DeviceInformationBridgeWindows::getMemoryStatus() {
	std::optional<MemoryStatus> totalSystemMemory;

	if(!totalSystemMemory.has_value()) {
		MEMORYSTATUSEX memoryStatus;
		memoryStatus.dwLength = sizeof(memoryStatus);

		if(GlobalMemoryStatusEx(&memoryStatus)) {
			totalSystemMemory = {
				memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys,
				memoryStatus.ullTotalPhys
			};
		}
	}

	return totalSystemMemory.value();
}

std::vector<std::string> DeviceInformationBridgeWindows::getMemoryDetails() {
	static const std::string MANUFACTURER_PROPERTY_NAME("Manufacturer");
	static const std::string MEMORY_TYPE_PROPERTY_NAME("MemoryType");
	static const std::string SMBIOS_MEMORY_TYPE_PROPERTY_NAME("SMBIOSMemoryType");
	static const std::string SPEED_PROPERTY_NAME("Speed");
	static const std::string CAPACITY_PROPERTY_NAME("Capacity");
	static const std::string PART_NUMBER_PROPERTY_NAME("PartNumber");
	static const std::vector<std::string> PHYSICAL_MEMORY_PROPERTY_NAMES = {
		MANUFACTURER_PROPERTY_NAME,
		MEMORY_TYPE_PROPERTY_NAME,
		SMBIOS_MEMORY_TYPE_PROPERTY_NAME,
		SPEED_PROPERTY_NAME,
		CAPACITY_PROPERTY_NAME,
		PART_NUMBER_PROPERTY_NAME
	};

	static std::vector<std::string> s_memoryDetails;

	if(s_memoryDetails.empty()) {
		std::optional<std::vector<std::map<std::string, std::any>>> optionalPhysicalMemoryInfo(WindowsUtilities::getWindowsManagementInstrumentationEntries(PHYSICAL_MEMORY_PROVIDER_CLASS_NAME, PHYSICAL_MEMORY_PROPERTY_NAMES));

		if(optionalPhysicalMemoryInfo.has_value()) {
			for(std::vector<std::map<std::string, std::any>>::const_iterator i = optionalPhysicalMemoryInfo->cbegin(); i != optionalPhysicalMemoryInfo->cend(); ++i) {
				const std::map<std::string, std::any> & memoryStickInfo = *i;

				uint64_t capacityBytes = Utilities::parseUnsignedLong(std::any_cast<std::string>(memoryStickInfo.at(CAPACITY_PROPERTY_NAME))).value_or(0U);
				std::string manufacturer(std::any_cast<std::string>(memoryStickInfo.at(MANUFACTURER_PROPERTY_NAME)));
				std::string partNumber(Utilities::trimString(std::any_cast<std::string>(memoryStickInfo.at(PART_NUMBER_PROPERTY_NAME))));
				MemoryType type = getMemoryTypeFromWindowsMemoryType(magic_enum::enum_cast<WindowsMemoryType>(std::any_cast<int32_t>(memoryStickInfo.at(MEMORY_TYPE_PROPERTY_NAME))).value_or(WindowsMemoryType::Unknown));
				int32_t speed = std::any_cast<int32_t>(memoryStickInfo.at(SPEED_PROPERTY_NAME));

				if(type == MemoryType::Unknown) {
					std::map<std::string, std::any>::const_iterator rawMemoryTypeIterator(memoryStickInfo.find(SMBIOS_MEMORY_TYPE_PROPERTY_NAME));

					if(rawMemoryTypeIterator != memoryStickInfo.end()) {
						type = magic_enum::enum_cast<MemoryType>(std::any_cast<int32_t>(rawMemoryTypeIterator->second)).value_or(MemoryType::Unknown);
					}
				}

				std::string memoryTypeAndSpeed;

				if(type != MemoryType::Unknown) {
					memoryTypeAndSpeed = fmt::format("{}-{} ", getMemoryTypeName(type), speed);
				}

				s_memoryDetails.emplace_back(fmt::format("{} {}{} GB ({})", manufacturer, memoryTypeAndSpeed, capacityBytes / 1073741824UL, partNumber));
			}
		}
	}

	return s_memoryDetails;
}

Dimension DeviceInformationBridgeWindows::getScreenResolution() {
	RECT desktopRectangle;
	const HWND desktopWindowHandle = GetDesktopWindow();
	GetWindowRect(desktopWindowHandle, &desktopRectangle);

	return Dimension(desktopRectangle.right, desktopRectangle.bottom);
}

std::string DeviceInformationBridgeWindows::getTimeZone() {
	DYNAMIC_TIME_ZONE_INFORMATION dynamicTimeZoneInfo;

	if(GetDynamicTimeZoneInformation(&dynamicTimeZoneInfo) == TIME_ZONE_ID_INVALID) {
		return {};
	}

	return Utilities::wideStringToString(dynamicTimeZoneInfo.StandardName);
}

std::string DeviceInformationBridgeWindows::getLocale() {
	LCID localeIdentifier = GetThreadLocale();
	wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

	if(LCIDToLocaleName(localeIdentifier, localeName, LOCALE_NAME_MAX_LENGTH, 0) == 0) {
		return {};
	}

	return Utilities::wideStringToString(localeName);
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
	static const std::string ADAPTER_NET_CONNECTION_STATUS_PROPERTY_NAME("NetConnectionStatus");
	static const std::string INVALID_MANUFACTURER_VALUE("Microsoft");
	static const int32_t CONNECTED_STATUS_VALUE = 2;
	static const std::array<std::pair<std::string, DeviceInformationBridge::NetworkConnectionType>, 4> NET_CONNECTION_ID_PREFIX_TYPES = {
		std::make_pair("Local Area Connection", NetworkConnectionType::Wired),
		std::make_pair("Ethernet", NetworkConnectionType::Wired),
		std::make_pair("Wireless Network Connection", NetworkConnectionType::Wireless),
		std::make_pair("Wi-Fi", NetworkConnectionType::Wireless)
	};
	static const std::vector<std::string> NETWORK_ADAPTER_PROPERTY_NAMES = {
		ADAPTER_DEVICE_ID_PROPERTY_NAME,
		ADAPTER_MAC_ADDRESS_PROPERTY_NAME,
		ADAPTER_MANUFACTURER_PROPERTY_NAME,
		ADAPTER_NAME_PROPERTY_NAME,
		ADAPTER_NET_CONNECTION_ID_PROPERTY_NAME,
		ADAPTER_SERVICE_NAME_PROPERTY_NAME,
		ADAPTER_PHYSICAL_ADAPTER_PROPERTY_NAME,
		ADAPTER_NET_CONNECTION_STATUS_PROPERTY_NAME
	};

	std::optional<std::vector<std::map<std::string, std::any>>> networkData(WindowsUtilities::getWindowsManagementInstrumentationEntries(NETWORK_ADAPTER_PROVIDER_CLASS_NAME, NETWORK_ADAPTER_PROPERTY_NAMES));

	if(!networkData.has_value()) {
		return {};
	}

	std::vector<NetworkAdapterInformation> networkAdapterInfoCollection;

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

			bool netConnectionIDTypeFound = false;

			for(auto j = NET_CONNECTION_ID_PREFIX_TYPES.cbegin(); j != NET_CONNECTION_ID_PREFIX_TYPES.cend(); ++j) {
				if(netConnectionIDValue.find(j->first) == 0) {
					networkAdapterInfo.type = j->second;
					netConnectionIDTypeFound = true;
					break;
				}
			}

			if(!netConnectionIDTypeFound) {
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

		std::map<std::string, std::any>::const_iterator netConnectionStatus(networkAdapterData.find(ADAPTER_NET_CONNECTION_STATUS_PROPERTY_NAME));

		if(netConnectionStatus != networkAdapterData.end() && netConnectionStatus->second.has_value()) {
			networkAdapterInfo.connected = std::any_cast<int32_t>(netConnectionStatus->second) == CONNECTED_STATUS_VALUE;
		}

		networkAdapterInfoCollection.emplace_back(networkAdapterInfo);
	}

	return networkAdapterInfoCollection;
}

DeviceInformationBridge::NetworkConnectionStatus DeviceInformationBridgeWindows::getNetworkConnectionStatus() {
	NetworkConnectionStatus networkConnectionStatus = NetworkConnectionStatus::Unknown;

	try {
		if(SUCCEEDED(CoInitialize(nullptr))) {
			INetworkListManager * networkListManager;

			if(SUCCEEDED(CoCreateInstance(CLSID_NetworkListManager, nullptr, CLSCTX_ALL, __uuidof(INetworkListManager), reinterpret_cast<LPVOID *>(&networkListManager)))) {
				NLM_CONNECTIVITY nlmConnectivity = NLM_CONNECTIVITY::NLM_CONNECTIVITY_DISCONNECTED;
				VARIANT_BOOL internetConnected = VARIANT_FALSE;

				if(SUCCEEDED(networkListManager->get_IsConnectedToInternet(&internetConnected))) {
					if(internetConnected == VARIANT_TRUE) {
						networkConnectionStatus = NetworkConnectionStatus::Internet;
					}
					else {
						networkConnectionStatus = NetworkConnectionStatus::Disconnected;
					}
				}

				if(!internetConnected) {
					std::vector<DeviceInformationBridge::NetworkAdapterInformation> networkAdapterInfo(getNetworkAdapterInformation());

					for(std::vector<DeviceInformationBridge::NetworkAdapterInformation>::const_iterator i = networkAdapterInfo.cbegin(); i != networkAdapterInfo.end(); ++i) {
						if(!i->connected) {
							continue;
						}

						networkConnectionStatus = NetworkConnectionStatus::Local;

						break;
					}
				}

				networkListManager->Release();
			}
		}

		CoUninitialize();
	}
	catch(std::runtime_error error) {
		networkConnectionStatus = NetworkConnectionStatus::Error;
	}

	return networkConnectionStatus;
}

DeviceInformationBridge::MemoryType DeviceInformationBridgeWindows::getMemoryTypeFromWindowsMemoryType(WindowsMemoryType memoryType) {
	switch(memoryType) {
		case WindowsMemoryType::Unknown:
			break;

		case WindowsMemoryType::Other:
			return MemoryType::Other;

		case WindowsMemoryType::DRAM:
			return MemoryType::DRAM;

		case WindowsMemoryType::SynchronousDRAM:
			return MemoryType::Other;

		case WindowsMemoryType::CacheDRAM:
			return MemoryType::Other;

		case WindowsMemoryType::EDO:
			return MemoryType::Other;

		case WindowsMemoryType::EDRAM:
			return MemoryType::EDRAM;

		case WindowsMemoryType::VRAM:
			return MemoryType::VRAM;

		case WindowsMemoryType::SRAM:
			return MemoryType::SRAM;

		case WindowsMemoryType::RAM:
			return MemoryType::RAM;

		case WindowsMemoryType::ROM:
			return MemoryType::ROM;

		case WindowsMemoryType::Flash:
			return MemoryType::Flash;

		case WindowsMemoryType::EEPROM:
			return MemoryType::EEPROM;

		case WindowsMemoryType::FEPROM:
			return MemoryType::FEPROM;

		case WindowsMemoryType::EPROM:
			return MemoryType::EPROM;

		case WindowsMemoryType::CDRAM:
			return MemoryType::CDRAM;

		case WindowsMemoryType::RAM3D:
			return MemoryType::RAM3D;

		case WindowsMemoryType::SDRAM:
			return MemoryType::SDRAM;

		case WindowsMemoryType::SGRAM:
			return MemoryType::SGRAM;

		case WindowsMemoryType::RDRAM:
			return MemoryType::RDRAM;

		case WindowsMemoryType::DDR:
			return MemoryType::DDR;

		case WindowsMemoryType::DDR2:
			return MemoryType::DDR2;

		case WindowsMemoryType::DDR2FBDIMM:
			return MemoryType::DDR2FBDIMM;

		case WindowsMemoryType::DDR3:
			return MemoryType::DDR3;

		case WindowsMemoryType::FBD2:
			return MemoryType::FBD2;

		case WindowsMemoryType::DDR4:
			return MemoryType::DDR4;

		case WindowsMemoryType::LPDDR:
			return MemoryType::LPDDR;

		case WindowsMemoryType::LPDDR2:
			return MemoryType::LPDDR2;

		case WindowsMemoryType::LPDDR3:
			return MemoryType::LPDDR3;

		case WindowsMemoryType::LPDDR4:
			return MemoryType::LPDDR4;

		case WindowsMemoryType::LogicalNonVolatileDevice:
			return MemoryType::LogicalNonVolatileDevice;

		case WindowsMemoryType::HBM:
			return MemoryType::HBM;

		case WindowsMemoryType::HBM2:
			return MemoryType::HBM2;

		case WindowsMemoryType::DDR5:
			return MemoryType::DDR5;

		case WindowsMemoryType::LPDDR5:
			return MemoryType::LPDDR5;
	}

	return MemoryType::Unknown;
}

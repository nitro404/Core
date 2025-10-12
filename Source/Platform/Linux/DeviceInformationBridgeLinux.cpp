#include "DeviceInformationBridgeLinux.h"

#include "Platform/Linux/LinuxUtilities.cpp"
#include "Utilities/StringUtilities.h"

#include <spdlog/spdlog.h>

#include <unistd.h>

#include <fstream>

static std::string_view chassisTypeToString(uint8_t chassisType) {
	switch(chassisType) {
		case 1:
			return "Other";
		case 2:
			return "Unknown";
		case 3:
			return "Desktop";
		case 4:
			return "Low Profile Desktop";
		case 5:
			return "Pizza Box";
		case 6:
			return "Mini Tower";
		case 7:
			return "Tower";
		case 8:
			return "Portable";
		case 9:
			return "Laptop";
		case 10:
			return "Notebook";
		case 11:
			return "Handheld";
		case 12:
			return "Docking Station";
		case 13:
			return "All-in-One";
		case 14:
			return "Sub-Notebook";
		case 15:
			return "Space-Saving";
		case 16:
			return "Lunch Box";
		case 17:
			return "Main System Chassis";
		case 18:
			return "Expansion Chassis";
		case 19:
			return "SubChassis";
		case 20:
			return "Bus Expansion Chassis";
		case 21:
			return "Peripheral Chassis";
		case 22:
			return "Storage Chassis";
		case 23:
			return "Rack Mount Chassis";
		case 24:
			return "Sealed-Case PC";
		case 25:
			return "Multi-System Chassis";
		case 26:
			return "Compact PCI";
		case 27:
			return "Advanced TCA";
		case 28:
			return "Blade Chassis";
		case 29:
			return "Blade Enclosure";
		case 30:
			return "Rack Mount Blade Chassis";
		case 31:
			return "Sealed-Case Blade PC";
		case 32:
			return "Tablet";
		case 33:
			return "Convertible";
		case 34:
			return "Detachable";
		case 35:
			return "IoT Gateway";
		case 36:
			return "Embedded";
		default:
			break;
	}

	return "";
}

static std::string getProcessorInformationEntryValue(std::string_view keyName) {
	static const std::string PROCESSOR_INFORMATION_FILE_PATH("/proc/cpuinfo");

	std::ifstream fileStream(PROCESSOR_INFORMATION_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read processor information from file: '{}'.", PROCESSOR_INFORMATION_FILE_PATH);
		return "";
	}

	std::string line;

	while(std::getline(fileStream, line)) {
		// Only parse information from the first CPU core, entries are separated by an empty line.
		if(line.empty()) {
			break;
		}

		size_t separatorIndex = line.find_first_of(":");

		if(separatorIndex == std::string::npos) {
			continue;
		}

		std::string_view rawCurrentKeyName(line.data(), separatorIndex);
		size_t keyNameEndIndex = rawCurrentKeyName.find_last_not_of("\t ");

		if(keyNameEndIndex == std::string::npos) {
			keyNameEndIndex = separatorIndex - 1;
		}

		std::string_view currentKeyName(rawCurrentKeyName.data(), keyNameEndIndex + 1);

		if(!Utilities::areStringsEqualIgnoreCase(currentKeyName, keyName)) {
			continue;
		}

		std::string_view rawCurrentValue(line.data() + separatorIndex + 1);
		size_t valueStartIndex = rawCurrentValue.find_first_not_of("\t ");

		return std::string(rawCurrentValue.data() + valueStartIndex, rawCurrentValue.length() - valueStartIndex);
	}

	spdlog::error("Could not find processor information entry for key: '{}'.", keyName);

	return "";
}

static std::string getOperatingSystemReleaseEntryValue(std::string_view keyName) {
	static const std::string OPERATING_SYSTEM_RELEASE_INFORMATION_FILE_PATH("/etc/os-release");

	std::ifstream fileStream(OPERATING_SYSTEM_RELEASE_INFORMATION_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read operating system release information from file: '{}'.", OPERATING_SYSTEM_RELEASE_INFORMATION_FILE_PATH);
		return "";
	}

	std::string line;

	while(std::getline(fileStream, line)) {
		size_t separatorIndex = line.find_first_of("=");

		if(separatorIndex == std::string::npos) {
			continue;
		}

		std::string_view rawCurrentKeyName(line.data(), separatorIndex);
		size_t keyNameEndIndex = rawCurrentKeyName.find_last_not_of("\t ");

		if(keyNameEndIndex == std::string::npos) {
			keyNameEndIndex = separatorIndex - 1;
		}

		std::string_view currentKeyName(rawCurrentKeyName.data(), keyNameEndIndex + 1);

		if(!Utilities::areStringsEqualIgnoreCase(currentKeyName, keyName)) {
			continue;
		}

		std::string_view rawCurrentValue(line.data() + separatorIndex + 1);
		size_t valueStartIndex = rawCurrentValue.find_first_not_of("\t \"");
		size_t valueEndIndex = rawCurrentValue.find_last_not_of("\t \"");

		return std::string(rawCurrentValue.data() + valueStartIndex, valueEndIndex - valueStartIndex + 1);
	}

	spdlog::error("Could not find operating system release information entry for key: '{}'.", keyName);

	return "";
}

static std::vector<uint64_t> getMemoryInformationEntryValues(const std::vector<std::string> & keyNames) {
	static const std::string MEMORY_INFORMATION_FILE_PATH("/proc/meminfo");

	if(keyNames.empty()) {
		return {};
	}

	std::vector<uint64_t> keyValues(keyNames.size(), std::numeric_limits<uint64_t>::max());
	std::ifstream fileStream(MEMORY_INFORMATION_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read memory information from file: '{}'.", MEMORY_INFORMATION_FILE_PATH);
		return keyValues;
	}

	std::string line;

	while(std::getline(fileStream, line)) {
		size_t separatorIndex = line.find_first_of(":");

		if(separatorIndex == std::string::npos) {
			continue;
		}

		std::string_view rawCurrentKeyName(line.data(), separatorIndex);
		size_t keyNameEndIndex = rawCurrentKeyName.find_last_not_of("\t ");

		if(keyNameEndIndex == std::string::npos) {
			keyNameEndIndex = separatorIndex - 1;
		}

		std::string_view currentKeyName(rawCurrentKeyName.data(), keyNameEndIndex + 1);

		size_t keyNameIndex = std::numeric_limits<size_t>::max();

		for(size_t i = 0; i < keyNames.size(); i++) {
			if(Utilities::areStringsEqualIgnoreCase(currentKeyName, keyNames[i])) {
				keyNameIndex = i;

				break;
			}
		}

		if(keyNameIndex == std::numeric_limits<size_t>::max()) {
			continue;
		}

		std::string_view rawCurrentValue(line.data() + separatorIndex + 1);
		size_t valueStartIndex = rawCurrentValue.find_first_not_of("\t ");
		size_t valueEndIndex = rawCurrentValue.find_last_not_of("\t kB");
		std::string_view currentValue(rawCurrentValue.data() + valueStartIndex, valueEndIndex - valueStartIndex + 1);
		std::optional<uint64_t> optionalCurrentValueInKilobytes(Utilities::parseUnsignedLong(std::string(currentValue)));

		if(!optionalCurrentValueInKilobytes.has_value()) {
			spdlog::error("Failed to parse memory size from value: '{}'.", currentValue);
			continue;
		}

		keyValues[keyNameIndex] = optionalCurrentValueInKilobytes.value() * 1024;

		bool allKeysFound = true;

		for(uint64_t keyValue : keyValues) {
			if(keyValue == std::numeric_limits<uint64_t>::max()) {
				allKeysFound = false;

				break;
			}
		}

		if(allKeysFound) {
			break;
		}
	}

	return keyValues;
}

DeviceInformationBridgeLinux::DeviceInformationBridgeLinux() { }

DeviceInformationBridgeLinux::~DeviceInformationBridgeLinux() { }

std::string DeviceInformationBridgeLinux::getHostName() {
	char hostName[HOST_NAME_MAX];

	if(!LinuxUtilities::isSuccess(gethostname(hostName, HOST_NAME_MAX), "Failed to get host name")) {
		return "";
	}

	return hostName;
}

std::string DeviceInformationBridgeLinux::getDeviceModel() {
	static const std::string PRODUCT_NAME_FILE_PATH("/sys/class/dmi/id/product_name");
	static const std::string INVALID_PRODUCT_NAME("System Product Name");

	static std::optional<std::string> s_optionalProductName;

	if(s_optionalProductName.has_value()) {
		return s_optionalProductName.value();
	}

	std::ifstream fileStream(PRODUCT_NAME_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read product name from file: '{}'.", PRODUCT_NAME_FILE_PATH);
		return "";
	}

	std::string productName;

	if(!std::getline(fileStream, productName)) {
		spdlog::error("Failed to read product name from file stream.");
		return "";
	}

	if(productName == INVALID_PRODUCT_NAME) {
		return "";
	}

	s_optionalProductName = std::move(productName);

	return s_optionalProductName.value();
}

std::string DeviceInformationBridgeLinux::getDeviceModelIdentifier() {
	// There is no equivalent Linux functionality to obtain this information.
	return "";
}

std::string DeviceInformationBridgeLinux::getDeviceType() {
	static const std::string CHASSIS_TYPE_FILE_PATH("/sys/class/dmi/id/chassis_type");

	static std::optional<std::string> s_optionalDeviceTypeName;

	if(s_optionalDeviceTypeName.has_value()) {
		return s_optionalDeviceTypeName.value();
	}

	std::ifstream fileStream(CHASSIS_TYPE_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read chassis type from file: '{}'.", CHASSIS_TYPE_FILE_PATH);
		return "";
	}

	std::string line;

	if(!std::getline(fileStream, line)) {
		spdlog::error("Failed to read chassis type from file stream.");
		return "";
	}

	std::optional<uint8_t> optionalChassisType(Utilities::parseUnsignedByte(line));

	if(!optionalChassisType.has_value()) {
		spdlog::error("Failed to parse chassis type from file data: '{}'.", line);
		return "";
	}

	std::string chassisTypeName(chassisTypeToString(optionalChassisType.value()));

	s_optionalDeviceTypeName = std::move(chassisTypeName);

	return s_optionalDeviceTypeName.value();
}

std::string DeviceInformationBridgeLinux::getDeviceManufacturerName() {
	static const std::string SYSTEM_VENDOR_FILE_PATH("/sys/class/dmi/id/sys_vendor");

	static std::optional<std::string> s_optionalSystemVendorName;

	if(s_optionalSystemVendorName.has_value()) {
		return s_optionalSystemVendorName.value();
	}

	std::ifstream fileStream(SYSTEM_VENDOR_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read system vendor name from file: '{}'.", SYSTEM_VENDOR_FILE_PATH);
		return "";
	}

	std::string systemVendorName;

	if(!std::getline(fileStream, systemVendorName)) {
		spdlog::error("Failed to read system vendor name from file stream.");
		return "";
	}

	s_optionalSystemVendorName = std::move(systemVendorName);

	return s_optionalSystemVendorName.value();
}

std::string DeviceInformationBridgeLinux::getDeviceUniqueIdentifier() {
	static const std::string MACHINE_IDENTIFIER_FILE_PATH("/etc/machine-id");

	static std::optional<std::string> s_optionalMachineIdentifier;

	if(s_optionalMachineIdentifier.has_value()) {
		return s_optionalMachineIdentifier.value();
	}

	std::ifstream fileStream(MACHINE_IDENTIFIER_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read machine identifier from file: '{}'.", MACHINE_IDENTIFIER_FILE_PATH);
		return "";
	}

	std::string machineIdentifier;

	if(!std::getline(fileStream, machineIdentifier)) {
		spdlog::error("Failed to read machine identifier from file stream.");
		return "";
	}

	s_optionalMachineIdentifier = std::move(machineIdentifier);

	return s_optionalMachineIdentifier.value();
}

std::string DeviceInformationBridgeLinux::getOperatingSystemName() {
	if(!populateSystemInformation()) {
		return "";
	}

	return m_systemInformation->sysname;
}

std::string DeviceInformationBridgeLinux::getOperatingSystemVersion() {
	static const std::string VERSION_IDENTIFIER_ENTRY_KEY("VERSION_ID");

	static std::optional<std::string> s_optionalOperatingSystemReleaseVersionIdentifier;

	if(s_optionalOperatingSystemReleaseVersionIdentifier.has_value()) {
		return s_optionalOperatingSystemReleaseVersionIdentifier.value();
	}

	std::string operatingSystemReleaseVersionIdentifier(getOperatingSystemReleaseEntryValue(VERSION_IDENTIFIER_ENTRY_KEY));

	if(operatingSystemReleaseVersionIdentifier.empty()) {
		return "";
	}

	s_optionalOperatingSystemReleaseVersionIdentifier = std::move(operatingSystemReleaseVersionIdentifier);

	return s_optionalOperatingSystemReleaseVersionIdentifier.value();
}

std::string DeviceInformationBridgeLinux::getArchitectureName() {
	if(!populateSystemInformation()) {
		return "";
	}

	return m_systemInformation->machine;
}

std::string DeviceInformationBridgeLinux::getProcessorName() {
	static const std::string MODEL_NAME_ENTRY_KEY("Model Name");

	static std::optional<std::string> s_optionalProcessorName;

	if(s_optionalProcessorName.has_value()) {
		return s_optionalProcessorName.value();
	}

	std::string processorName(getProcessorInformationEntryValue(MODEL_NAME_ENTRY_KEY));

	if(processorName.empty()) {
		return "";
	}

	s_optionalProcessorName = std::move(processorName);

	return s_optionalProcessorName.value();
}

std::string DeviceInformationBridgeLinux::getMotherboardName() {
	static const std::string MOTHERBOARD_NAME_FILE_PATH("/sys/class/dmi/id/board_name");

	static std::optional<std::string> s_optionalMotherboardName;

	if(s_optionalMotherboardName.has_value()) {
		return s_optionalMotherboardName.value();
	}

	std::ifstream fileStream(MOTHERBOARD_NAME_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read motherboard name from file: '{}'.", MOTHERBOARD_NAME_FILE_PATH);
		return "";
	}

	std::string motherboardName;

	if(!std::getline(fileStream, motherboardName)) {
		spdlog::error("Failed to read motherboard name from file stream.");
		return "";
	}

	s_optionalMotherboardName = std::move(motherboardName);

	return s_optionalMotherboardName.value();
}

std::vector<std::string> DeviceInformationBridgeLinux::getGraphicsCardNames() {
	// Note: This function is not yet implemented.
	return {};
}

DeviceInformationBridge::MemoryStatus DeviceInformationBridgeLinux::getMemoryStatus() {
	static const std::vector<std::string> MEMORY_INFORMATION_ENTRY_KEYS({"MemTotal", "MemFree", "MemAvailable", "Cached"});

	std::vector<uint64_t> memoryInformationValues(getMemoryInformationEntryValues(MEMORY_INFORMATION_ENTRY_KEYS));

	DeviceInformationBridge::MemoryStatus memoryStatus;
	memoryStatus.total = memoryInformationValues[0];

	if(memoryInformationValues[2] != std::numeric_limits<uint64_t>::max()) {
		memoryStatus.used = memoryInformationValues[0] - memoryInformationValues[2];
	}
	else {
		memoryStatus.used = memoryInformationValues[0] - (memoryInformationValues[1] + memoryInformationValues[3]);
	}

	return memoryStatus;
}

std::vector<std::string> DeviceInformationBridgeLinux::getMemoryDetails() {
	// Note: This function is not yet implemented.
	return {};
}

Dimension DeviceInformationBridgeLinux::getScreenResolution() {
	// Note: This function is not yet implemented.
	return {};
}

std::string DeviceInformationBridgeLinux::getTimeZone() {
	static const std::string TIME_ZONE_FILE_PATH("/etc/timezone");

	std::ifstream fileStream(TIME_ZONE_FILE_PATH);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read time zone from file: '{}'.", TIME_ZONE_FILE_PATH);
		return "";
	}

	std::string timeZone;

	if(!std::getline(fileStream, timeZone)) {
		spdlog::error("Failed to read time zone from file stream.");
		return "";
	}

	return timeZone;
}

std::string DeviceInformationBridgeLinux::getLocale() {
	const char * rawLocale = getenv("LANG");

	if(rawLocale == nullptr) {
		return "";
	}

	std::string_view locale(rawLocale);

	size_t localeSeparatorIndex = locale.find_first_of(".");

	if(localeSeparatorIndex == std::string::npos) {
		return std::string(locale);
	}

	return std::string(locale.data(), localeSeparatorIndex);
}

std::string DeviceInformationBridgeLinux::getMACAddress(NetworkConnectionType connectionType) {
	// Note: This function is not yet implemented.
	return "";
}

std::vector<DeviceInformationBridge::NetworkAdapterInformation> DeviceInformationBridgeLinux::getNetworkAdapterInformation() {
	// Note: This function is not yet implemented.
	return {};
}

DeviceInformationBridge::NetworkConnectionStatus DeviceInformationBridgeLinux::getNetworkConnectionStatus() {
	// Note: This function is not yet implemented.
	return DeviceInformationBridge::NetworkConnectionStatus::Unknown;
}

bool DeviceInformationBridgeLinux::populateSystemInformation() {
	if(m_systemInformation.has_value()) {
		return true;
	}

	utsname systemInformation;
	std::memset(&systemInformation, 0, sizeof(utsname));

	if(!LinuxUtilities::isSuccess(uname(&systemInformation), "Failed to get system information")) {
		return false;
	}

	m_systemInformation = std::move(systemInformation);

	return true;
}

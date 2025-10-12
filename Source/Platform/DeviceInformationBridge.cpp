#include "DeviceInformationBridge.h"

#include "Utilities/StringUtilities.h"

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

DeviceInformationBridge::DeviceInformationBridge() { }

DeviceInformationBridge::~DeviceInformationBridge() { }

std::optional<DeviceInformationBridge::OperatingSystemType> DeviceInformationBridge::getOperatingSystemType() {
	static std::optional<DeviceInformationBridge::OperatingSystemType> s_optionalOperatingSystemType;

	if(!s_optionalOperatingSystemType.has_value()) {
		std::string operatingSystemName(getOperatingSystemName());
		std::string operatingSystemNameLowerCase(Utilities::toLowerCase(operatingSystemName));

		if(operatingSystemNameLowerCase.find("windows") != std::string::npos) {
			s_optionalOperatingSystemType = OperatingSystemType::Windows;
		}
		else if(operatingSystemNameLowerCase.find("linux") != std::string::npos) {
			s_optionalOperatingSystemType = OperatingSystemType::Linux;
		}
		else {
			spdlog::error("Unknown operating system type: '{}'.", operatingSystemName);
		}
	}

	return s_optionalOperatingSystemType;
}

std::optional<DeviceInformationBridge::ArchitectureType> DeviceInformationBridge::getArchitectureType() {
	static std::optional<DeviceInformationBridge::ArchitectureType> s_optionalArchitectureType;

	if(!s_optionalArchitectureType.has_value()) {
		std::string architectureName(getArchitectureName());

		if(Utilities::areStringsEqualIgnoreCase(architectureName, "mips")) {
			s_optionalArchitectureType = ArchitectureType::MIPS32;
		}
		else if(Utilities::areStringsEqualIgnoreCase(architectureName, "ppc")) {
			s_optionalArchitectureType = ArchitectureType::PPC32;
		}
		else if(Utilities::areStringsEqualIgnoreCase(architectureName, "sparc")) {
			s_optionalArchitectureType = ArchitectureType::SPARC32;
		}
		else {
			constexpr auto architectureEntries = magic_enum::enum_entries<ArchitectureType>();

			for(const auto & currentArchitectureEntry : architectureEntries) {
				if(Utilities::areStringsEqualIgnoreCase(currentArchitectureEntry.second, architectureName)) {
					s_optionalArchitectureType = currentArchitectureEntry.first;
					break;
				}
			}
		}

		if(!s_optionalArchitectureType.has_value()) {
			if(architectureName.find("64") != std::string::npos) {
				s_optionalArchitectureType = ArchitectureType::x64;
			}
			else if(architectureName.find("32") != std::string::npos) {
				s_optionalArchitectureType = ArchitectureType::x86;
			}
		}
	}

	return s_optionalArchitectureType;
}

bool DeviceInformationBridge::isConnectedToInternet() {
	return getNetworkConnectionStatus() == NetworkConnectionStatus::Internet;
}

std::string DeviceInformationBridge::getMemoryTypeName(MemoryType memoryType) {
	switch(memoryType) {
		case MemoryType::Unknown:
			break;

		case MemoryType::Other:
			return "Other";

		case MemoryType::DRAM:
			return "DRAM";

		case MemoryType::EDRAM:
			return "EDRAM";

		case MemoryType::VRAM:
			return "VRAM";

		case MemoryType::SRAM:
			return "SRAM";

		case MemoryType::RAM:
			return "RAM";

		case MemoryType::ROM:
			return "ROM";

		case MemoryType::Flash:
			return "Flash";

		case MemoryType::EEPROM:
			return "EEPROM";

		case MemoryType::FEPROM:
			return "FEPROM";

		case MemoryType::EPROM:
			return "EPROM";

		case MemoryType::CDRAM:
			return "CDRAM";

		case MemoryType::RAM3D:
			return "3DRAM";

		case MemoryType::SDRAM:
			return "SDRAM";

		case MemoryType::SGRAM:
			return "SGRAM";

		case MemoryType::RDRAM:
			return "RDRAM";

		case MemoryType::DDR:
			return "DDR";

		case MemoryType::DDR2:
			return "DDR2";

		case MemoryType::DDR2FBDIMM:
			return "DDR2 FB-DIMM";

		case MemoryType::DDR3:
			return "DDR3";

		case MemoryType::FBD2:
			return "FBD2";

		case MemoryType::DDR4:
			return "DDR4";

		case MemoryType::LPDDR:
			return "LPDDR";

		case MemoryType::LPDDR2:
			return "LPDDR2";

		case MemoryType::LPDDR3:
			return "LPDDR3";

		case MemoryType::LPDDR4:
			return "LPDDR4";

		case MemoryType::LogicalNonVolatileDevice:
			return "Logical Non-Volatile Device";

		case MemoryType::HBM:
			return "HBM";

		case MemoryType::HBM2:
			return "HBM2";

		case MemoryType::DDR5:
			return "DDR5";

		case MemoryType::LPDDR5:
			return "LPDDR5";
	}

	return "Unknown";
}

#ifndef _DEVICE_INFORMATION_BRIDGE_H_
#define _DEVICE_INFORMATION_BRIDGE_H_

#include "Dimension.h"
#include "Singleton/Singleton.h"

#include <optional>
#include <string>
#include <vector>

class DeviceInformationBridge : public Singleton<DeviceInformationBridge> {
public:
	// SMBIOS Reference Specification 7.18.2
	enum class MemoryType : uint8_t {
		Unknown = 1,
		Other = 2,
		DRAM = 3,
		EDRAM = 4,
		VRAM = 5,
		SRAM = 6,
		RAM = 7,
		ROM = 8,
		Flash = 9,
		EEPROM = 10,
		FEPROM = 11,
		EPROM = 12,
		CDRAM = 13,
		RAM3D = 14,
		SDRAM = 15,
		SGRAM = 16,
		RDRAM = 17,
		DDR = 18,
		DDR2 = 19,
		DDR2FBDIMM = 20,
		DDR3 = 24,
		FBD2 = 25,
		DDR4 = 26,
		LPDDR = 27,
		LPDDR2 = 28,
		LPDDR3 = 29,
		LPDDR4 = 30,
		LogicalNonVolatileDevice = 31,
		HBM = 32,
		HBM2 = 33,
		DDR5 = 34,
		LPDDR5 = 35
	};

	enum class NetworkConnectionType {
		Wired,
		Wireless
	};

	enum class NetworkConnectionStatus {
		Internet,
		Local,
		Disconnected,
		Error,
		Unknown
	};

	enum class OperatingSystemType {
		Windows,
		Linux,
		MacOS
	};

	enum class ArchitectureType {
		x86,
		x64,
		Alpha,
		ARMV7L,
		AARCH64,
		HPPA,
		i386,
		i486,
		i586,
		i686,
		i860,
		MIPS32,
		MIPS64,
		PPC32,
		PPC64,
		RISCV,
		s390x,
		SH4,
		SPARC32,
		SPARC64
	};

	struct MemoryStatus {
		uint64_t used = 0;
		uint64_t total = 0;
	};

	struct NetworkAdapterInformation {
		std::string id;
		std::string name;
		NetworkConnectionType type = NetworkConnectionType::Wired;
		std::string manufacturer;
		std::string macAddress;
		std::string serviceName;
		bool connected = false;
	};

	virtual ~DeviceInformationBridge();

	virtual std::string getHostName() = 0;
	virtual std::string getDeviceModel() = 0;
	virtual std::string getDeviceModelIdentifier() = 0;
	virtual std::string getDeviceType() = 0;
	virtual std::string getDeviceManufacturerName() = 0;
	virtual std::string getDeviceUniqueIdentifier() = 0;
	virtual std::string getOperatingSystemName() = 0;
	std::optional<OperatingSystemType> getOperatingSystemType();
	virtual std::string getOperatingSystemVersion() = 0;
	virtual std::string getArchitectureName() = 0;
	std::optional<ArchitectureType> getArchitectureType();
	virtual std::string getProcessorName() = 0;
	virtual std::string getMotherboardName() = 0;
	virtual std::vector<std::string> getGraphicsCardNames() = 0;
	virtual MemoryStatus getMemoryStatus() = 0;
	virtual std::vector<std::string> getMemoryDetails() = 0;
	virtual Dimension getScreenResolution() = 0;
	virtual std::string getTimeZone() = 0;
	virtual std::string getLocale() = 0;
	virtual std::string getMACAddress(NetworkConnectionType connectionType = NetworkConnectionType::Wired) = 0;
	virtual std::vector<NetworkAdapterInformation> getNetworkAdapterInformation() = 0;
	bool isConnectedToInternet();
	virtual NetworkConnectionStatus getNetworkConnectionStatus() = 0;

	static std::string getMemoryTypeName(MemoryType memoryType);

protected:
	DeviceInformationBridge();

private:
	DeviceInformationBridge(const DeviceInformationBridge &) = delete;
	const DeviceInformationBridge & operator = (const DeviceInformationBridge &) = delete;
};

#endif // _DEVICE_INFORMATION_BRIDGE_H_

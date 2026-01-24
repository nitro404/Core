#ifndef _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_
#define _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_

#include "Platform/DeviceInformationBridge.h"

class DeviceInformationBridgeWindows final : public DeviceInformationBridge {
public:
	DeviceInformationBridgeWindows();
	~DeviceInformationBridgeWindows() override;

	virtual std::string getHostName() override;
	virtual std::string getDeviceModel() override;
	virtual std::string getDeviceModelIdentifier() override;
	virtual std::string getDeviceType() override;
	virtual std::string getDeviceManufacturerName() override;
	virtual std::string getDeviceUniqueIdentifier() override;
	virtual std::string getOperatingSystemName() override;
	virtual std::string getOperatingSystemVersion() override;
	virtual std::string getArchitectureName() override;
	virtual std::string getProcessorName() override;
	virtual std::string getMotherboardName() override;
	virtual std::vector<std::string> getGraphicsCardNames() override;
	virtual MemoryStatus getMemoryStatus() override;
	virtual std::vector<std::string> getMemoryDetails() override;
	virtual Dimension getScreenResolution() override;
	virtual std::string getTimeZone() override;
	virtual std::string getLocale() override;
	virtual std::string getMACAddress(NetworkConnectionType connectionType = NetworkConnectionType::Wired) override;
	virtual std::vector<NetworkAdapterInformation> getNetworkAdapterInformation() override;
	virtual NetworkConnectionStatus getNetworkConnectionStatus() override;

private:
	enum class WindowsMemoryType : uint8_t {
		Unknown = 0,
		Other = 1,
		DRAM = 2,
		SynchronousDRAM = 3,
		CacheDRAM = 4,
		EDO = 5,
		EDRAM = 6,
		VRAM = 7,
		SRAM = 8,
		RAM = 9,
		ROM = 10,
		Flash = 11,
		EEPROM = 12,
		FEPROM = 13,
		EPROM = 14,
		CDRAM = 15,
		RAM3D = 16,
		SDRAM = 17,
		SGRAM = 18,
		RDRAM = 19,
		DDR = 20,
		DDR2 = 21,
		DDR2FBDIMM = 22,
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

	static MemoryType getMemoryTypeFromWindowsMemoryType(WindowsMemoryType memoryType);

	DeviceInformationBridgeWindows(const DeviceInformationBridgeWindows &) = delete;
	const DeviceInformationBridgeWindows & operator = (const DeviceInformationBridgeWindows &) = delete;
};

#endif // _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_

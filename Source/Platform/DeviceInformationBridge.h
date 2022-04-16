#ifndef _DEVICE_INFORMATION_BRIDGE_H_
#define _DEVICE_INFORMATION_BRIDGE_H_

#include "Dimension.h"
#include <string>
#include <vector>

class DeviceInformationBridge {
public:
	enum class NetworkConnectionType {
		Wired,
		Wireless
	};

	struct MemoryStatus {
		uint64_t used;
		uint64_t total;
	};

	struct NetworkAdapterInformation {
		std::string id;
		std::string name;
		NetworkConnectionType type = NetworkConnectionType::Wired;
		std::string manufacturer;
		std::string macAddress;
		std::string serviceName;
	};

	DeviceInformationBridge();
	virtual ~DeviceInformationBridge();

	virtual std::string getHostName() = 0;
	virtual std::string getDeviceModel() = 0;
	virtual std::string getDeviceModelIdentifier() = 0;
	virtual std::string getDeviceType() = 0;
	virtual std::string getDeviceManufacturerName() = 0;
	virtual std::string getDeviceUniqueIdentifier() = 0;
	virtual std::string getOperatingSystemName() = 0;
	virtual std::string getOperatingSystemVersion() = 0;
	virtual std::string getOperatingSystemArchitecture() = 0;
	virtual std::string getProcessorName() = 0;
	virtual std::string getMotherboardName() = 0;
	virtual std::vector<std::string> getGraphicsCardNames() = 0;
	virtual MemoryStatus getMemoryStatus() = 0;
	virtual Dimension getScreenResolution() = 0;
	virtual std::string getTimeZone() = 0;
	virtual std::string getLocale() = 0;
	virtual std::string getMACAddress(NetworkConnectionType connectionType = NetworkConnectionType::Wired) = 0;
	virtual std::vector<NetworkAdapterInformation> getNetworkAdapterInformation() = 0;


private:
	DeviceInformationBridge(const DeviceInformationBridge &) = delete;
	DeviceInformationBridge(DeviceInformationBridge &&) noexcept = delete;
	const DeviceInformationBridge & operator = (const DeviceInformationBridge &) = delete;
	const DeviceInformationBridge & operator = (DeviceInformationBridge &&) noexcept = delete;
};

#endif // _DEVICE_INFORMATION_BRIDGE_H_

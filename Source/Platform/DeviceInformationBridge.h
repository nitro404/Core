#ifndef _DEVICE_INFORMATION_BRIDGE_H_
#define _DEVICE_INFORMATION_BRIDGE_H_

#include <string>
#include <vector>

class DeviceInformationBridge {
public:
	enum class NetworkConnectionType {
		Wired,
		Wireless
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
	virtual std::string getModel() = 0;
	virtual std::string getModelIdentifier() = 0;
	virtual std::string getManufacturerName() = 0;
	virtual std::string getUniqueIdentifier() = 0;
	virtual std::string getOperatingSystemName() = 0;
	virtual std::string getOperatingSystemVersion() = 0;
	virtual std::string getOperatingSystemArchitecture() = 0;
	virtual std::string getMACAddress(NetworkConnectionType connectionType = NetworkConnectionType::Wired) = 0;
	virtual std::vector<NetworkAdapterInformation> getNetworkAdapterInformation() = 0;

	static const std::string UNKNOWN_VALUE;

private:
	DeviceInformationBridge(const DeviceInformationBridge &) = delete;
	DeviceInformationBridge(DeviceInformationBridge &&) noexcept = delete;
	const DeviceInformationBridge & operator = (const DeviceInformationBridge &) = delete;
	const DeviceInformationBridge & operator = (DeviceInformationBridge &&) noexcept = delete;
};

#endif // _DEVICE_INFORMATION_BRIDGE_H_

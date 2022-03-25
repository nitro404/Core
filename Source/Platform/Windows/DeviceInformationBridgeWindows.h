#ifndef _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_
#define _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_

#include "Platform/DeviceInformationBridge.h"

class DeviceInformationBridgeWindows final : public DeviceInformationBridge {
public:
	DeviceInformationBridgeWindows();
	virtual ~DeviceInformationBridgeWindows();

	virtual std::string getHostName() override;
	virtual std::string getModel() override;
	virtual std::string getModelIdentifier() override;
	virtual std::string getManufacturerName() override;
	virtual std::string getUniqueIdentifier() override;
	virtual std::string getOperatingSystemName() override;
	virtual std::string getOperatingSystemVersion() override;
	virtual std::string getOperatingSystemArchitecture() override;
	virtual std::string getMACAddress(NetworkConnectionType connectionType = NetworkConnectionType::Wired) override;
	virtual std::vector<NetworkAdapterInformation> getNetworkAdapterInformation() override;

private:
	DeviceInformationBridgeWindows(const DeviceInformationBridgeWindows &) = delete;
	DeviceInformationBridgeWindows(DeviceInformationBridgeWindows &&) noexcept = delete;
	const DeviceInformationBridgeWindows & operator = (const DeviceInformationBridgeWindows &) = delete;
	const DeviceInformationBridgeWindows & operator = (DeviceInformationBridgeWindows &&) noexcept = delete;
};

#endif // _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_

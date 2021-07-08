#ifndef _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_
#define _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_

#include "Platform/DeviceInformationBridge.h"

class DeviceInformationBridgeWindows : public DeviceInformationBridge {
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
};

#endif // _DEVICE_INFORMATION_BRIDGE_WINDOWS_H_

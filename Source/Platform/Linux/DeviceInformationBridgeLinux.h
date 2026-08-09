#ifndef _DEVICE_INFORMATION_BRIDGE_LINUX_H_
#define _DEVICE_INFORMATION_BRIDGE_LINUX_H_

#include "Platform/DeviceInformationBridge.h"

#include <sys/utsname.h>

#include <optional>

class DeviceInformationBridgeLinux final : public DeviceInformationBridge {
	friend class FactoryRegistry;

public:
	~DeviceInformationBridgeLinux() override;

	// DeviceInformationBridge Virtuals
	std::string getHostName() override;
	std::string getDeviceModel() override;
	std::string getDeviceModelIdentifier() override;
	std::string getDeviceType() override;
	std::string getDeviceManufacturerName() override;
	std::string getDeviceUniqueIdentifier() override;
	std::string getOperatingSystemName() override;
	std::string getOperatingSystemVersion() override;
	std::string getArchitectureName() override;
	std::string getProcessorName() override;
	std::string getMotherboardName() override;
	std::vector<std::string> getGraphicsCardNames() override;
	MemoryStatus getMemoryStatus() override;
	std::vector<std::string> getMemoryDetails() override;
	Dimension getScreenResolution() override;
	std::string getTimeZone() override;
	std::string getLocale() override;
	std::string getMACAddress(NetworkConnectionType connectionType = NetworkConnectionType::Wired) override;
	std::vector<NetworkAdapterInformation> getNetworkAdapterInformation() override;
	NetworkConnectionStatus getNetworkConnectionStatus() override;

private:
	DeviceInformationBridgeLinux();

	bool populateSystemInformation();

	std::optional<utsname> m_systemInformation;

	DeviceInformationBridgeLinux(const DeviceInformationBridgeLinux &) = delete;
	const DeviceInformationBridgeLinux & operator = (const DeviceInformationBridgeLinux &) = delete;
};

#endif // _DEVICE_INFORMATION_BRIDGE_LINUX_H_

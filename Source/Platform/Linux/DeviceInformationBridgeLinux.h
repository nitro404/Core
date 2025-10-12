#ifndef _DEVICE_INFORMATION_BRIDGE_LINUX_H_
#define _DEVICE_INFORMATION_BRIDGE_LINUX_H_

#include "Platform/DeviceInformationBridge.h"

#include <sys/utsname.h>

#include <optional>

class DeviceInformationBridgeLinux final : public DeviceInformationBridge {
public:
	DeviceInformationBridgeLinux();
	virtual ~DeviceInformationBridgeLinux();

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
	bool populateSystemInformation();

	std::optional<utsname> m_systemInformation;

	DeviceInformationBridgeLinux(const DeviceInformationBridgeLinux &) = delete;
	const DeviceInformationBridgeLinux & operator = (const DeviceInformationBridgeLinux &) = delete;
};

#endif // _DEVICE_INFORMATION_BRIDGE_LINUX_H_

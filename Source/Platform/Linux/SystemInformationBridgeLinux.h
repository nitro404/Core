#ifndef _SYSTEM_INFORMATION_BRIDGE_LINUX_H_
#define _SYSTEM_INFORMATION_BRIDGE_LINUX_H_

#include "Platform/SystemInformationBridge.h"

class SystemInformationBridgeLinux final : public SystemInformationBridge {
	friend class FactoryRegistry;

public:
	~SystemInformationBridgeLinux() override;

	std::optional<std::string> getHomeDirectoryPath() override;
	std::optional<std::string> getApplicationDataDirectoryPath() override;

private:
	SystemInformationBridgeLinux();

	SystemInformationBridgeLinux(const SystemInformationBridgeLinux &) = delete;
	const SystemInformationBridgeLinux & operator = (const SystemInformationBridgeLinux &) = delete;
};

#endif // _SYSTEM_INFORMATION_BRIDGE_LINUX_H_

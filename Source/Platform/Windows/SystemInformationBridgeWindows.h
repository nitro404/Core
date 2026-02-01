#ifndef _SYSTEM_INFORMATION_BRIDGE_WINDOWS_H_
#define _SYSTEM_INFORMATION_BRIDGE_WINDOWS_H_

#include "Platform/SystemInformationBridge.h"

class SystemInformationBridgeWindows final : public SystemInformationBridge {
public:
	SystemInformationBridgeWindows();
	~SystemInformationBridgeWindows() override;

	std::optional<std::string> getHomeDirectoryPath() override;
	std::optional<std::string> getApplicationDataDirectoryPath() override;

private:
	SystemInformationBridgeWindows(const SystemInformationBridgeWindows &) = delete;
	const SystemInformationBridgeWindows & operator = (const SystemInformationBridgeWindows &) = delete;
};

#endif // _SYSTEM_INFORMATION_BRIDGE_WINDOWS_H_

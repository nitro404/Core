#ifndef _SYSTEM_INFORMATION_BRIDGE_H_
#define _SYSTEM_INFORMATION_BRIDGE_H_

#include "Singleton/Singleton.h"

#include <optional>
#include <string>

class SystemInformationBridge : public Singleton<SystemInformationBridge> {
public:
	~SystemInformationBridge() override;

	virtual std::optional<std::string> getHomeDirectoryPath() = 0;
	virtual std::optional<std::string> getApplicationDataDirectoryPath() = 0;

protected:
	SystemInformationBridge();

private:
	SystemInformationBridge(const SystemInformationBridge &) = delete;
	const SystemInformationBridge & operator = (const SystemInformationBridge &) = delete;
};

#endif // _SYSTEM_INFORMATION_BRIDGE_H_

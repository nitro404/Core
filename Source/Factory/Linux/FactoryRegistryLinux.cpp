#include "Factory/FactoryRegistry.h"

#include "Logging/Linux/LogSystemLinux.h"
#include "Platform/Linux/DeviceInformationBridgeLinux.h"
#include "Platform/Linux/ProcessCreatorLinux.h"
#include "Platform/Linux/SystemInformationBridgeLinux.h"

void FactoryRegistry::assignPlatformFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setFactory<LogSystem>([]() {
		return std::unique_ptr<LogSystemLinux>(new LogSystemLinux());
	});

	setFactory<DeviceInformationBridge>([]() {
		return std::unique_ptr<DeviceInformationBridgeLinux>(new DeviceInformationBridgeLinux());
	});

	setFactory<ProcessCreator>([]() {
		return std::unique_ptr<ProcessCreatorLinux>(new ProcessCreatorLinux());
	});

	setFactory<SystemInformationBridge>([]() {
		return std::unique_ptr<SystemInformationBridgeLinux>(new SystemInformationBridgeLinux());
	});
}

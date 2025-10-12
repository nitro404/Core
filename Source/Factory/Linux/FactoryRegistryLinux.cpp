#include "Factory/FactoryRegistry.h"

#include "Logging/Linux/LogSystemLinux.h"
#include "Platform/Linux/DeviceInformationBridgeLinux.h"
#include "Platform/Linux/ProcessCreatorLinux.h"

void FactoryRegistry::assignPlatformFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setFactory<LogSystem>([]() {
		return std::make_unique<LogSystemLinux>();
	});

	setFactory<DeviceInformationBridge>([]() {
		return std::make_unique<DeviceInformationBridgeLinux>();
	});

	setFactory<ProcessCreator>([]() {
		return std::make_unique<ProcessCreatorLinux>();
	});
}

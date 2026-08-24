#include "Factory/FactoryRegistry.h"

#include "Platform/DeviceInformationBridge.h"
#include "Platform/ProcessCreator.h"
#include "Platform/SystemInformationBridge.h"

void FactoryRegistry::assignPlatformFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setFactory<DeviceInformationBridge>([]() {
		return std::unique_ptr<DeviceInformationBridge>(nullptr);
	});

	setFactory<ProcessCreator>([]() {
		return std::unique_ptr<ProcessCreator>(nullptr);
	});

	setFactory<SystemInformationBridge>([]() {
		return std::unique_ptr<SystemInformationBridge>(nullptr);
	});
}

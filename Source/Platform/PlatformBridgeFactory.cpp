#include "PlatformBridgeFactory.h"

#if WINDOWS

#include "Platform/Windows/DeviceInformationBridgeWindows.h"

#endif

DeviceInformationBridge * PlatformBridgeFactory::createDeviceInformationBridge() {
#if WINDOWS
	return new DeviceInformationBridgeWindows();
#else
	return nullptr;
#endif
}

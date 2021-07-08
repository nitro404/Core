#ifndef _PLATFORM_BRIDGE_FACTORY_H_
#define _PLATFORM_BRIDGE_FACTORY_H_

#include "DeviceInformationBridge.h"

class PlatformBridgeFactory {
public:
	static DeviceInformationBridge * createDeviceInformationBridge();

private:
	PlatformBridgeFactory() = delete;
};

#endif // _PLATFORM_BRIDGE_FACTORY_H_

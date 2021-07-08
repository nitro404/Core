#include "DeviceInformationBridge.h"

const std::string DeviceInformationBridge::UNKNOWN_VALUE("Unknown");

DeviceInformationBridge::NetworkAdapterInformation::NetworkAdapterInformation()
	: type(DeviceInformationBridge::NetworkConnectionType::Wired) { }

DeviceInformationBridge::DeviceInformationBridge() = default;

DeviceInformationBridge::~DeviceInformationBridge() = default;

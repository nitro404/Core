#include "SegmentAnalytics.h"

#include "Location/GeoLocationService.h"
#include "Network/IPAddressService.h"
#include "Platform/DeviceInformationBridge.h"
#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <date/tz.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <chrono>

SegmentAnalytics::Configuration::~Configuration() { }

SegmentAnalytics::LibraryInfoProvider::~LibraryInfoProvider() { }

SegmentAnalytics::SegmentAnalytics()
	: m_initialized(false)
	, m_started(false)
	, m_includeIPAddress(false)
	, m_includeGeoLocation(false)
	, m_batchMode(true)
	, m_maxEventQueueSize(std::numeric_limits<uint16_t>::max())
	, m_dataStorage(std::make_unique<DataStorage>()) { }

SegmentAnalytics::~SegmentAnalytics() { }

bool SegmentAnalytics::isInitialized() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_initialized;
}

bool SegmentAnalytics::initialize(const Configuration & configuration) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_initialized || !isConfigurationValid(configuration)) {
		return false;
	}

	if(getLibraryInfoProvider() == nullptr) {
		spdlog::error("Library info provider must not be null!");
		return false;
	}

	m_writeKey = configuration.writeKey;
	m_includeIPAddress = configuration.includeIPAddress;
	m_includeGeoLocation = configuration.includeGeoLocation;
	m_batchMode = configuration.batchMode;
	m_maxEventQueueSize = configuration.maxEventQueueSize;
	m_applicationName = configuration.applicationName;
	m_applicationVersion = configuration.applicationVersion;
	m_applicationBuild = configuration.applicationBuild;
	m_applicationPackageName = configuration.applicationPackageName;
	m_userAgent = configuration.userAgent;

	if(m_includeIPAddress) {
		IPAddressService * ipAddressService = IPAddressService::getInstance();

		m_ipAddress = ipAddressService->getIPAddress(IPAddressService::IPAddressType::V6);
	}

	if(m_includeGeoLocation) {
		GeoLocationService * geoLocationService = GeoLocationService::getInstance();

		if(geoLocationService->isInitialized()) {
			m_geoLocation = geoLocationService->getGeoLocation();
		}
	}

	if(!m_dataStorage->initialize(configuration.dataStorageFilePath, m_applicationVersion, m_applicationBuild)) {
		spdlog::error("Failed to initialize Segment analytics data storage!");
		return false;
	}

	m_sessionStartTimePoint = std::chrono::system_clock::now();

	m_initialized = true;

	DeviceInformationBridge * deviceInfoBridge = DeviceInformationBridge::getInstance();

	if(m_dataStorage->isFirstApplicationLaunch() || m_dataStorage->getAnonymousID().empty()) {
		m_anonymousID = deviceInfoBridge->getDeviceUniqueIdentifier();

		m_dataStorage->setAnonymousID(m_anonymousID);
	}
	else {
		m_anonymousID = m_dataStorage->getAnonymousID();
	}

	return true;
}

bool SegmentAnalytics::isStarted() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_started;
}

bool SegmentAnalytics::start() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized || m_started) {
		return false;
	}

	if(shouldTrackApplicationEvents()) {
		if(m_dataStorage->isFirstApplicationLaunch()) {
			onApplicationInstalled();
		}
		else if(m_dataStorage->wasApplicationUpdated()) {
			onApplicationUpdated();
		}

		onApplicationOpened();
	}

	if(m_dataStorage->hasUserID()) {
		queueEvent(SegmentAnalyticEvent::createIdentifyEvent(m_dataStorage->getUserID(), m_dataStorage->getUserTraits()));
	}

	m_started = true;

	return true;
}

void SegmentAnalytics::stop() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_started = false;
}

bool SegmentAnalytics::isFirstApplicationLaunch() const {
	return m_dataStorage->isFirstApplicationLaunch();
}

uint64_t SegmentAnalytics::getSessionNumber() const {
	return m_dataStorage->getSessionNumber();
}

bool SegmentAnalytics::wasApplicationUpdated() const {
	return m_dataStorage->wasApplicationUpdated();
}

std::string SegmentAnalytics::getPreviousApplicationVersion() const {
	return m_dataStorage->getPreviousApplicationVersion();
}

std::string SegmentAnalytics::getPreviousApplicationBuild() const {
	return m_dataStorage->getPreviousApplicationBuild();
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> SegmentAnalytics::getSessionStartedTimePoint() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_sessionStartTimePoint;
}

std::optional<std::chrono::milliseconds> SegmentAnalytics::getSessionDuration() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_sessionStartTimePoint.has_value()) {
		return {};
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_sessionStartTimePoint.value());
}

bool SegmentAnalytics::shouldIncludeIPAddress() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_includeIPAddress;
}

const std::string & SegmentAnalytics::getIPAddress() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_ipAddress;
}

bool SegmentAnalytics::shouldIncludeGeoLocation() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_includeGeoLocation;
}

const std::optional<GeoLocation> & SegmentAnalytics::getGeoLocation() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_geoLocation;
}

bool SegmentAnalytics::isUsingBatchMode() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_batchMode;
}

uint16_t SegmentAnalytics::getMaxEventQueueSize() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_maxEventQueueSize;
}

std::string SegmentAnalytics::getAnonymousID() const {
	return m_anonymousID;
}

bool SegmentAnalytics::setIdentity(const std::string & userID, const std::map<std::string, std::any> & traits) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	m_dataStorage->setUserData(userID, traits);

	return queueEvent(SegmentAnalyticEvent::createIdentifyEvent(userID, traits));
}

bool SegmentAnalytics::setAlias(const std::string & previousUserID) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	return queueEvent(SegmentAnalyticEvent::createAliasEvent(previousUserID));
}

bool SegmentAnalytics::setGroup(const std::string & group, const std::map<std::string, std::any> & traits) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	return queueEvent(SegmentAnalyticEvent::createGroupEvent(group, traits));
}

bool SegmentAnalytics::track(const std::string & name, const std::map<std::string, std::any> & properties) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	return queueEvent(SegmentAnalyticEvent::createTrackEvent(name, properties, m_dataStorage->getUserID(), m_dataStorage->getUserTraits()));
}

bool SegmentAnalytics::screen(const std::string & name, const std::map<std::string, std::any> & properties) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return screen(name, Utilities::emptyString, properties);
}

bool SegmentAnalytics::screen(const std::string & name, const std::string & category, const std::map<std::string, std::any> & properties) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	return queueEvent(SegmentAnalyticEvent::createScreenEvent(name, category, properties, m_dataStorage->getUserID(), m_dataStorage->getUserTraits()));
}

void SegmentAnalytics::reset() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_dataStorage->reset();
}

const std::string & SegmentAnalytics::getWriteKey() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_writeKey;
}

const SegmentAnalytics::DataStorage * SegmentAnalytics::getDataStorage() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_dataStorage.get();
}

SegmentAnalytics::DataStorage * SegmentAnalytics::getDataStorage() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_dataStorage.get();
}

const std::string & SegmentAnalytics::getApplicationName() const {
	return m_applicationName;
}

const std::string & SegmentAnalytics::getApplicationVersion() const {
	return m_applicationVersion;
}

const std::string & SegmentAnalytics::getApplicationBuild() const {
	return m_applicationBuild;
}

const std::string & SegmentAnalytics::getApplicationPackageName() const {
	return m_applicationPackageName;
}

const std::string & SegmentAnalytics::getUserAgent() const {
	return m_userAgent;
}

bool SegmentAnalytics::onApplicationInstalled() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	std::map<std::string, std::any> properties;
	properties["version"] = m_applicationVersion;
	properties["build"] = m_applicationBuild;
	return track("Application Installed", properties);
}

bool SegmentAnalytics::onApplicationUpdated() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	std::map<std::string, std::any> properties;
	properties["version"] = m_applicationVersion;
	properties["build"] = m_applicationBuild;
	properties["previous_version"] = m_dataStorage->getPreviousApplicationVersion();
	properties["previous_build"] = m_dataStorage->getPreviousApplicationBuild();
	return track("Application Updated", properties);
}

bool SegmentAnalytics::onApplicationOpened() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	std::map<std::string, std::any> properties;
	properties["from_background"] = false;
	return track("Application Opened", properties);
}

bool SegmentAnalytics::onApplicationClosed() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	std::optional<std::chrono::milliseconds> sessionDuration(getSessionDuration());

	if(!sessionDuration.has_value()) {
		return false;
	}

	std::map<std::string, std::any> properties;
	properties["duration"] = std::chrono::duration_cast<std::chrono::seconds>(sessionDuration.value()).count();
	return track("Application Closed", properties);
}

bool SegmentAnalytics::isConfigurationValid(const Configuration & configuration) const {
	if(configuration.writeKey.empty()) {
		spdlog::error("Invalid Segment analytics configuration - write key cannot be empty.");
		return false;
	}

	if(configuration.dataStorageFilePath.empty()) {
		spdlog::error("Invalid Segment analytics configuration - data storage file path cannot be empty.");
		return false;
	}

	if(configuration.batchMode && configuration.maxEventQueueSize <= 1) {
		spdlog::error("Invalid Segment analytics configuration - max event queue size value must be greater than one.");
		return false;
	}

	if(configuration.applicationName.empty()) {
		spdlog::error("Invalid Segment analytics configuration - application name cannot be empty.");
		return false;
	}

	if(configuration.applicationVersion.empty()) {
		spdlog::error("Invalid Segment analytics configuration - application version cannot be empty.");
		return false;
	}

	if(configuration.applicationBuild.empty()) {
		spdlog::error("Invalid Segment analytics configuration - application build cannot be empty.");
		return false;
	}

	return true;
}

std::unique_ptr<rapidjson::Document> SegmentAnalytics::createBaseEventPayloadDocument() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	// segment analytics reference documentation:
	// https://segment.com/docs/connections/spec/common
	// https://segment.com/docs/connections/sources/catalog/libraries/server/http-api

	std::unique_ptr<rapidjson::Document> payloadDocument(std::make_unique<rapidjson::Document>(rapidjson::kObjectType));
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = payloadDocument->GetAllocator();

	const LibraryInfoProvider * libraryInfoProvider = getLibraryInfoProvider();
	DeviceInformationBridge * deviceInfoBridge = DeviceInformationBridge::getInstance();

	Dimension screenResolution(deviceInfoBridge->getScreenResolution());
	DeviceInformationBridge::MemoryStatus memoryStatus(deviceInfoBridge->getMemoryStatus());
	static std::vector<std::string> memoryDetails(deviceInfoBridge->getMemoryDetails());
	static std::vector<std::string> graphicsCardNames(deviceInfoBridge->getGraphicsCardNames());
	std::vector<DeviceInformationBridge::NetworkAdapterInformation> networkAdapterInfo(deviceInfoBridge->getNetworkAdapterInformation());

	bool wiredAdapterConnected = false;
	bool wirelessAdapterConnected = false;

	for(std::vector<DeviceInformationBridge::NetworkAdapterInformation>::const_iterator i = networkAdapterInfo.cbegin(); i != networkAdapterInfo.end(); ++i) {
		if(!i->connected) {
			continue;
		}

		switch(i->type) {
			case DeviceInformationBridge::NetworkConnectionType::Wired: {
				wiredAdapterConnected = true;
				break;
			}

			case DeviceInformationBridge::NetworkConnectionType::Wireless: {
				wirelessAdapterConnected = true;
				break;
			}
		}
	}

	std::string timeZone;

	try {
		timeZone = date::current_zone()->name();
	}
	catch(std::runtime_error error) {
		spdlog::error("Failed to obtain current time zone name: {}", error.what());
	}

	if(timeZone.empty()) {
		timeZone = deviceInfoBridge->getTimeZone();
	}

	rapidjson::Value sentAtValue(Utilities::timePointToString(std::chrono::system_clock::now(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
	payloadDocument->AddMember(rapidjson::StringRef("sentAt"), sentAtValue, allocator);

	rapidjson::Value contextValue(rapidjson::kObjectType);

	rapidjson::Value ipAddressValue(m_includeIPAddress ? m_ipAddress.c_str() : "0.0.0.0", allocator);
	contextValue.AddMember(rapidjson::StringRef("ip"), ipAddressValue, allocator);
	rapidjson::Value localeValue(deviceInfoBridge->getLocale().c_str(), allocator);
	contextValue.AddMember(rapidjson::StringRef("locale"), localeValue, allocator);
	rapidjson::Value timeZoneValue(timeZone.c_str(), allocator);
	contextValue.AddMember(rapidjson::StringRef("timezone"), timeZoneValue, allocator);
	rapidjson::Value userAgentValue(m_userAgent.c_str(), allocator);
	contextValue.AddMember(rapidjson::StringRef("userAgent"), userAgentValue, allocator);
	contextValue.AddMember(rapidjson::StringRef("active"), rapidjson::Value(true), allocator);

	rapidjson::Value appValue(rapidjson::kObjectType);

	rapidjson::Value appNameValue(m_applicationName.c_str(), allocator);
	appValue.AddMember(rapidjson::StringRef("name"), appNameValue, allocator);
	rapidjson::Value appVersionValue(m_applicationVersion.c_str(), allocator);
	appValue.AddMember(rapidjson::StringRef("version"), appVersionValue, allocator);
	rapidjson::Value appBuildValue(m_applicationBuild.c_str(), allocator);
	appValue.AddMember(rapidjson::StringRef("build"), appBuildValue, allocator);

	if(!m_applicationPackageName.empty()) {
		rapidjson::Value appNamespaceValue(m_applicationPackageName.c_str(), allocator);
		appValue.AddMember(rapidjson::StringRef("namespace"), appNamespaceValue, allocator);
	}

	contextValue.AddMember(rapidjson::StringRef("app"), appValue, allocator);

	rapidjson::Value libraryValue(rapidjson::kObjectType);

	rapidjson::Value libraryNameValue(libraryInfoProvider->getLibraryName().c_str(), allocator);
	libraryValue.AddMember(rapidjson::StringRef("name"), libraryNameValue, allocator);
	rapidjson::Value libraryVersionValue(libraryInfoProvider->getLibraryVersion().c_str(), allocator);
	libraryValue.AddMember(rapidjson::StringRef("version"), libraryVersionValue, allocator);

	contextValue.AddMember(rapidjson::StringRef("library"), libraryValue, allocator);

	rapidjson::Value deviceValue(rapidjson::kObjectType);

	rapidjson::Value deviceIdentifierValue(deviceInfoBridge->getDeviceUniqueIdentifier().c_str(), allocator);
	deviceValue.AddMember(rapidjson::StringRef("id"), deviceIdentifierValue, allocator);
	rapidjson::Value deviceTypeValue(deviceInfoBridge->getDeviceType().c_str(), allocator);
	deviceValue.AddMember(rapidjson::StringRef("type"), deviceTypeValue, allocator);
	rapidjson::Value deviceNameValue(deviceInfoBridge->getDeviceModel().c_str(), allocator);
	deviceValue.AddMember(rapidjson::StringRef("name"), deviceNameValue, allocator);
	rapidjson::Value deviceModelValue(deviceInfoBridge->getDeviceModelIdentifier().c_str(), allocator);
	deviceValue.AddMember(rapidjson::StringRef("model"), deviceModelValue, allocator);
	rapidjson::Value deviceManufacturerValue(deviceInfoBridge->getDeviceManufacturerName().c_str(), allocator);
	deviceValue.AddMember(rapidjson::StringRef("manufacturer"), deviceManufacturerValue, allocator);
	deviceValue.AddMember(rapidjson::StringRef("adTrackingEnabled"), rapidjson::Value(false), allocator);
	deviceValue.AddMember(rapidjson::StringRef("usedMemory"), rapidjson::Value(memoryStatus.used), allocator);
	deviceValue.AddMember(rapidjson::StringRef("totalMemory"), rapidjson::Value(memoryStatus.total), allocator);
	rapidjson::Value processorValue(deviceInfoBridge->getProcessorName().c_str(), allocator);
	deviceValue.AddMember(rapidjson::StringRef("cpu"), processorValue, allocator);
	rapidjson::Value motherboardValue(deviceInfoBridge->getMotherboardName().c_str(), allocator);
	deviceValue.AddMember(rapidjson::StringRef("motherboard"), motherboardValue, allocator);

	rapidjson::Value gpuValue(rapidjson::kArrayType);

	for(std::vector<std::string>::const_iterator i = graphicsCardNames.cbegin(); i != graphicsCardNames.cend(); ++i) {
		rapidjson::Value graphicsCardNameValue(i->c_str(), allocator);
		gpuValue.PushBack(graphicsCardNameValue, allocator);
	}

	deviceValue.AddMember(rapidjson::StringRef("gpu"), gpuValue, allocator);

	rapidjson::Value ramValue(rapidjson::kArrayType);

	for(std::vector<std::string>::const_iterator i = memoryDetails.cbegin(); i != memoryDetails.cend(); ++i) {
		rapidjson::Value memoryStickValue(i->c_str(), allocator);
		ramValue.PushBack(memoryStickValue, allocator);
	}

	deviceValue.AddMember(rapidjson::StringRef("ram"), ramValue, allocator);

	contextValue.AddMember(rapidjson::StringRef("device"), deviceValue, allocator);

	rapidjson::Value operatingSystemValue(rapidjson::kObjectType);

	rapidjson::Value operatingSystemNameValue(deviceInfoBridge->getOperatingSystemName().c_str(), allocator);
	operatingSystemValue.AddMember(rapidjson::StringRef("name"), operatingSystemNameValue, allocator);
	rapidjson::Value operatingSystemVersionValue(deviceInfoBridge->getOperatingSystemVersion().c_str(), allocator);
	operatingSystemValue.AddMember(rapidjson::StringRef("version"), operatingSystemVersionValue, allocator);
	rapidjson::Value operatingSystemArchitectureValue(deviceInfoBridge->getOperatingSystemArchitectureName().c_str(), allocator);
	operatingSystemValue.AddMember(rapidjson::StringRef("architecture"), operatingSystemArchitectureValue, allocator);

	contextValue.AddMember(rapidjson::StringRef("os"), operatingSystemValue, allocator);

	rapidjson::Value screenValue(rapidjson::kObjectType);

	screenValue.AddMember(rapidjson::StringRef("width"), rapidjson::Value(screenResolution.w), allocator);
	screenValue.AddMember(rapidjson::StringRef("height"), rapidjson::Value(screenResolution.h), allocator);

	contextValue.AddMember(rapidjson::StringRef("screen"), screenValue, allocator);

	rapidjson::Value networkValue(rapidjson::kObjectType);

	networkValue.AddMember(rapidjson::StringRef("wifi"), rapidjson::Value(wirelessAdapterConnected), allocator);
	networkValue.AddMember(rapidjson::StringRef("wired"), rapidjson::Value(wiredAdapterConnected), allocator);
	networkValue.AddMember(rapidjson::StringRef("cellular"), rapidjson::Value(false), allocator);
	networkValue.AddMember(rapidjson::StringRef("bluetooth"), rapidjson::Value(false), allocator);
	networkValue.AddMember(rapidjson::StringRef("local"), rapidjson::Value(wiredAdapterConnected || wirelessAdapterConnected), allocator);
	networkValue.AddMember(rapidjson::StringRef("internet"), rapidjson::Value(deviceInfoBridge->isConnectedToInternet()), allocator);

	contextValue.AddMember(rapidjson::StringRef("network"), networkValue, allocator);

	if(m_geoLocation.has_value()) {
		rapidjson::Value locationValue(rapidjson::kObjectType);

		locationValue.AddMember(rapidjson::StringRef("latitude"), rapidjson::Value(m_geoLocation->latitude), allocator);
		locationValue.AddMember(rapidjson::StringRef("longitude"), rapidjson::Value(m_geoLocation->longitude), allocator);
		locationValue.AddMember(rapidjson::StringRef("speed"), rapidjson::Value(0), allocator);
		rapidjson::Value locationCityValue(m_geoLocation->cityName.c_str(), allocator);
		locationValue.AddMember(rapidjson::StringRef("city"), locationCityValue, allocator);
		rapidjson::Value locationRegionValue(m_geoLocation->regionName.c_str(), allocator);
		locationValue.AddMember(rapidjson::StringRef("region"), locationRegionValue, allocator);
		rapidjson::Value locationCountryValue(m_geoLocation->countryName.c_str(), allocator);
		locationValue.AddMember(rapidjson::StringRef("country"), locationCountryValue, allocator);

		contextValue.AddMember(rapidjson::StringRef("location"), locationValue, allocator);
	}

	payloadDocument->AddMember(rapidjson::StringRef("context"), contextValue, allocator);

	return payloadDocument;
}

bool SegmentAnalytics::addEventDataToValue(const SegmentAnalyticEvent & analyticEvent, const std::string & anonymousID, rapidjson::Value & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool batchMode) {
	if(!analyticEvent.isValid() || anonymousID.empty()) {
		return false;
	}

	SegmentAnalyticEvent::EventType eventType = analyticEvent.getType();

	if(!outputValue.IsObject()) {
		outputValue.SetObject();
	}

	std::string namePropertyName;

	if(eventType == SegmentAnalyticEvent::EventType::Track) {
		namePropertyName = "event";
	}
	else if(eventType == SegmentAnalyticEvent::EventType::Screen) {
		namePropertyName = "name";
	}

	rapidjson::Value anonymousIDValue(anonymousID.c_str(), allocator);
	outputValue.AddMember(rapidjson::StringRef("anonymousId"), anonymousIDValue, allocator);

	if(!namePropertyName.empty()) {
		rapidjson::Value nameValue(analyticEvent.getName().c_str(), allocator);
		rapidjson::Value namePropertyNameValue(namePropertyName.c_str(), allocator);
		outputValue.AddMember(namePropertyNameValue, nameValue, allocator);
	}

	if(batchMode) {
		std::string eventTypeName(Utilities::toLowerCase(magic_enum::enum_name(analyticEvent.getType())));
		rapidjson::Value typeValue(eventTypeName.c_str(), allocator);
		outputValue.AddMember(rapidjson::StringRef("type"), typeValue, allocator);
	}

	rapidjson::Value timeStampValue(Utilities::timePointToString(analyticEvent.getTimestamp(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
	outputValue.AddMember(rapidjson::StringRef("timestamp"), timeStampValue, allocator);

	if(analyticEvent.hasUserID()) {
		rapidjson::Value userIDValue(analyticEvent.getUserID().c_str(), allocator);
		outputValue.AddMember(rapidjson::StringRef("userId"), userIDValue, allocator);
	}

	if(!analyticEvent.getCategory().empty()) {
		rapidjson::Value categoryValue(analyticEvent.getCategory().c_str(), allocator);
		outputValue.AddMember(rapidjson::StringRef("category"), categoryValue, allocator);
	}

	if(analyticEvent.hasProperties()) {
		rapidjson::Value propertiesValue(Utilities::anyMapToJSONValue(analyticEvent.getProperties(), allocator, false));
		outputValue.AddMember(rapidjson::StringRef("properties"), propertiesValue, allocator);
	}

	if(analyticEvent.hasUserTraits()) {
		rapidjson::Value userTraitsValue(Utilities::anyMapToJSONValue(analyticEvent.getUserTraits(), allocator, false));
		outputValue.AddMember(rapidjson::StringRef("traits"), userTraitsValue, allocator);
	}

	return true;
}

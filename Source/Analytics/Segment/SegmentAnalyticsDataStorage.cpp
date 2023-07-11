#include "SegmentAnalytics.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <fstream>

static constexpr const char * JSON_SEGMENT_FILE_TYPE_PROPERTY_NAME = "fileType";
static constexpr const char * JSON_SEGMENT_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_SEGMENT_PREVIOUS_SESSION_NUMBER_PROPERTY_NAME = "previousSessionNumber";
static constexpr const char * JSON_SEGMENT_PREVIOUS_APPLICATION_VERSION_PROPERTY_NAME = "previousApplicationVersion";
static constexpr const char * JSON_SEGMENT_PREVIOUS_APPLICATION_BUILD_PROPERTY_NAME = "previousApplicationBuild";
static constexpr const char * JSON_SEGMENT_ANONYMOUS_ID_PROPERTY_NAME = "anonymousID";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_ID_COUNTER_PROPERTY_NAME = "analyticEventIDCounter";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENTS_PROPERTY_NAME = "analyticEvents";
static const std::array<std::string_view, 8> JSON_SEGMENT_PROPERTY_NAMES = {
	JSON_SEGMENT_FILE_TYPE_PROPERTY_NAME,
	JSON_SEGMENT_FILE_FORMAT_VERSION_PROPERTY_NAME,
	JSON_SEGMENT_PREVIOUS_SESSION_NUMBER_PROPERTY_NAME,
	JSON_SEGMENT_PREVIOUS_APPLICATION_VERSION_PROPERTY_NAME,
	JSON_SEGMENT_PREVIOUS_APPLICATION_BUILD_PROPERTY_NAME,
	JSON_SEGMENT_ANONYMOUS_ID_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENT_ID_COUNTER_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENTS_PROPERTY_NAME
};

const std::string SegmentAnalytics::DataStorage::FILE_TYPE("Segment Analytics Cache");
const std::string SegmentAnalytics::DataStorage::FILE_FORMAT_VERSION("1.0.0");

SegmentAnalytics::DataStorage::DataStorage()
	: m_initialized(false)
	, m_firstApplicationLaunch(true)
	, m_sessionNumber(1) { }

SegmentAnalytics::DataStorage::DataStorage(DataStorage && dataStorage) noexcept
	: m_initialized(dataStorage.m_initialized)
	, m_filePath(std::move(dataStorage.m_filePath))
	, m_firstApplicationLaunch(dataStorage.m_firstApplicationLaunch)
	, m_sessionNumber(dataStorage.m_sessionNumber)
	, m_previousApplicationVersion(std::move(dataStorage.m_previousApplicationVersion))
	, m_previousApplicationBuild(std::move(dataStorage.m_previousApplicationBuild))
	, m_anonymousID(std::move(dataStorage.m_anonymousID))
	, m_userID(std::move(dataStorage.m_userID))
	, m_applicationVersion(std::move(dataStorage.m_applicationVersion))
	, m_applicationBuild(std::move(dataStorage.m_applicationBuild))
	, m_userTraits(std::move(dataStorage.m_userTraits))
	, m_pendingAnalyticEvents(std::move(dataStorage.m_pendingAnalyticEvents)) { }

const SegmentAnalytics::DataStorage & SegmentAnalytics::DataStorage::operator = (DataStorage && dataStorage) noexcept {
	if(this != &dataStorage) {
		m_initialized = dataStorage.m_initialized;
		m_filePath = std::move(dataStorage.m_filePath);
		m_firstApplicationLaunch = dataStorage.m_firstApplicationLaunch;
		m_sessionNumber = dataStorage.m_sessionNumber;
		m_previousApplicationVersion = std::move(dataStorage.m_previousApplicationVersion);
		m_previousApplicationBuild = std::move(dataStorage.m_previousApplicationBuild);
		m_anonymousID = std::move(dataStorage.m_anonymousID);
		m_userID = std::move(dataStorage.m_userID);
		m_applicationVersion = std::move(dataStorage.m_applicationVersion);
		m_applicationBuild = std::move(dataStorage.m_applicationBuild);
		m_userTraits = std::move(dataStorage.m_userTraits);
		m_pendingAnalyticEvents = std::move(dataStorage.m_pendingAnalyticEvents);
	}

	return *this;
}

SegmentAnalytics::DataStorage::~DataStorage() { }

bool SegmentAnalytics::DataStorage::isInitialized() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_initialized;
}

bool SegmentAnalytics::DataStorage::initialize(const std::string & filePath, const std::string & applicationVersion, const std::string applicationBuild) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_initialized) {
		return false;
	}

	if(filePath.empty()) {
		spdlog::error("Failed to initialize Segment analytics data storage, file path cannot be empty!");
		return false;
	}

	if(applicationVersion.empty()) {
		spdlog::error("Failed to initialize Segment analytics data storage, application version cannot be empty!");
		return false;
	}

	if(applicationBuild.empty()) {
		spdlog::error("Failed to initialize Segment analytics data storage, application build cannot be empty!");
		return false;
	}

	m_filePath = filePath;
	m_applicationVersion = applicationVersion;
	m_applicationBuild = applicationBuild;

	if(!createRequiredDirectories()) {
		spdlog::error("Failed to create required Segment analytics cache directories!");
		return false;
	}

	load();
	save();

	m_initialized = true;

	return true;
}

std::string SegmentAnalytics::DataStorage::getFilePath() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_filePath;
}

bool SegmentAnalytics::DataStorage::isFirstApplicationLaunch() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_firstApplicationLaunch;
}

uint64_t SegmentAnalytics::DataStorage::getSessionNumber() const {
	return m_sessionNumber;
}

bool SegmentAnalytics::DataStorage::wasApplicationUpdated() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	std::optional<uint8_t> optionalVersionComparison(Utilities::compareVersions(m_previousApplicationVersion, m_applicationVersion));

	if(!optionalVersionComparison.has_value()) {
		return false;
	}

	return optionalVersionComparison.value() < 0;
}

std::string SegmentAnalytics::DataStorage::getPreviousApplicationVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_previousApplicationVersion;
}

std::string SegmentAnalytics::DataStorage::getPreviousApplicationBuild() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_previousApplicationBuild;
}

std::string SegmentAnalytics::DataStorage::getAnonymousID() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_anonymousID;
}

void SegmentAnalytics::DataStorage::setAnonymousID(const std::string & anonymousID) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_anonymousID = anonymousID;

	save();
}

bool SegmentAnalytics::DataStorage::hasUserID() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !m_userID.empty();
}

bool SegmentAnalytics::DataStorage::hasAnyUserTraits() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !m_userTraits.empty();
}

std::string SegmentAnalytics::DataStorage::getUserID() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_userID;
}

const std::map<std::string, std::any> & SegmentAnalytics::DataStorage::getUserTraits() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_userTraits;
}

void SegmentAnalytics::DataStorage::setUserID(const std::string & userID) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_userID = Utilities::trimString(userID);

	save();
}

void SegmentAnalytics::DataStorage::setUserTraits(const std::map<std::string, std::any> & traits) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	for(std::map<std::string, std::any>::const_iterator i = traits.cbegin(); i != traits.cend(); ++i) {
		if(!SegmentAnalyticEvent::isValidPropertyName(i->first)) {
			continue;
		}

		m_userTraits[Utilities::trimString(i->first)] = i->second;
	}

	save();
}

void SegmentAnalytics::DataStorage::setUserData(const std::string & userID, const std::map<std::string, std::any> & traits) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_userID = Utilities::trimString(userID);

	for(std::map<std::string, std::any>::const_iterator i = traits.cbegin(); i != traits.cend(); ++i) {
		if(!SegmentAnalyticEvent::isValidPropertyName(i->first)) {
			continue;
		}

		m_userTraits[Utilities::trimString(i->first)] = i->second;
	}

	save();
}

void SegmentAnalytics::DataStorage::clearUserData() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_userID.clear();
	m_userTraits.clear();

	save();
}

bool SegmentAnalytics::DataStorage::hasAnyPendingAnalyticEvents() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !m_pendingAnalyticEvents.empty();
}

size_t SegmentAnalytics::DataStorage::numberOfPendingAnalyticEvents() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_pendingAnalyticEvents.size();
}

bool SegmentAnalytics::DataStorage::hasPendingAnalyticEvent(const SegmentAnalyticEvent & analyticEvent) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return hasPendingAnalyticEventWithID(analyticEvent.getID());
}

bool SegmentAnalytics::DataStorage::hasPendingAnalyticEventWithID(uint64_t analyticEventID) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_pendingAnalyticEvents.find(analyticEventID) != m_pendingAnalyticEvents.cend();
}

std::shared_ptr<SegmentAnalyticEvent> SegmentAnalytics::DataStorage::getPendingAnalyticEventWithID(uint64_t analyticEventID) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::map<uint64_t, std::shared_ptr<SegmentAnalyticEvent>>::const_iterator analyticEvent = m_pendingAnalyticEvents.find(analyticEventID);

	if(analyticEvent == m_pendingAnalyticEvents.end()) {
		return nullptr;
	}

	return analyticEvent->second;
}

std::vector<std::shared_ptr<SegmentAnalyticEvent>> SegmentAnalytics::DataStorage::getPendingAnalyticEvents() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::vector<std::shared_ptr<SegmentAnalyticEvent>> pendingAnalyticEvents;

	for(std::map<uint64_t, std::shared_ptr<SegmentAnalyticEvent>>::const_iterator i = m_pendingAnalyticEvents.cbegin(); i != m_pendingAnalyticEvents.cend(); ++i) {
		pendingAnalyticEvents.push_back(i->second);
	}

	return pendingAnalyticEvents;
}

bool SegmentAnalytics::DataStorage::addPendingAnalyticEvent(std::shared_ptr<SegmentAnalyticEvent> analyticEvent) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized || !SegmentAnalyticEvent::isValid(analyticEvent.get())) {
		return false;
	}

	m_pendingAnalyticEvents[analyticEvent->getID()] = analyticEvent;

	save();

	return true;
}

bool SegmentAnalytics::DataStorage::removePendingAnalyticEvent(const SegmentAnalyticEvent & analyticEvent) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return removePendingAnalyticEventWithID(analyticEvent.getID());
}

bool SegmentAnalytics::DataStorage::removePendingAnalyticEventWithID(uint64_t analyticEventID) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized || !hasPendingAnalyticEventWithID(analyticEventID)) {
		return false;
	}

	m_pendingAnalyticEvents.erase(analyticEventID);

	save();

	return true;
}

size_t SegmentAnalytics::DataStorage::removePendingAnalyticEvents(const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized || analyticEvents.empty()) {
		return 0;
	}

	size_t numberOfPendingEventsRemoved = 0;

	for(std::vector<std::shared_ptr<SegmentAnalyticEvent>>::const_iterator i = analyticEvents.cbegin(); i != analyticEvents.cend(); ++i) {
		if(!hasPendingAnalyticEventWithID((*i)->getID())) {
			continue;
		}

		m_pendingAnalyticEvents.erase((*i)->getID());
	}

	save();

	return numberOfPendingEventsRemoved;
}

bool SegmentAnalytics::DataStorage::clearPendingAnalyticEvents() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	m_pendingAnalyticEvents.clear();

	save();

	return true;
}

void SegmentAnalytics::DataStorage::reset() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_userID.clear();
	m_userTraits.clear();

	save();
}

rapidjson::Document SegmentAnalytics::DataStorage::toJSON() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	rapidjson::Document dataDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = dataDocument.GetAllocator();

	rapidjson::Value fileTypeVersionValue(FILE_TYPE.c_str(), allocator);
	dataDocument.AddMember(rapidjson::StringRef(JSON_SEGMENT_FILE_TYPE_PROPERTY_NAME), fileTypeVersionValue, allocator);

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	dataDocument.AddMember(rapidjson::StringRef(JSON_SEGMENT_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	dataDocument.AddMember(rapidjson::StringRef(JSON_SEGMENT_PREVIOUS_SESSION_NUMBER_PROPERTY_NAME), rapidjson::Value(m_sessionNumber), allocator);

	rapidjson::Value previousApplicationVersionValue(m_applicationVersion.c_str(), allocator);
	dataDocument.AddMember(rapidjson::StringRef(JSON_SEGMENT_PREVIOUS_APPLICATION_VERSION_PROPERTY_NAME), previousApplicationVersionValue, allocator);

	rapidjson::Value previousApplicationBuildValue(m_applicationBuild.c_str(), allocator);
	dataDocument.AddMember(rapidjson::StringRef(JSON_SEGMENT_PREVIOUS_APPLICATION_BUILD_PROPERTY_NAME), previousApplicationBuildValue, allocator);

	rapidjson::Value anonymousIDValue(m_anonymousID.c_str(), allocator);
	dataDocument.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANONYMOUS_ID_PROPERTY_NAME), anonymousIDValue, allocator);

	dataDocument.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_ID_COUNTER_PROPERTY_NAME), rapidjson::Value(SegmentAnalyticEvent::getIDCounter()), allocator);

	rapidjson::Value analyticEventsValue(rapidjson::kArrayType);
	analyticEventsValue.Reserve(m_pendingAnalyticEvents.size(), allocator);

	for(std::map<uint64_t, std::shared_ptr<SegmentAnalyticEvent>>::const_iterator i = m_pendingAnalyticEvents.cbegin(); i != m_pendingAnalyticEvents.cend(); ++i) {
		analyticEventsValue.PushBack(i->second->toJSON(allocator), allocator);
	}

	dataDocument.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENTS_PROPERTY_NAME), analyticEventsValue, allocator);

	return dataDocument;
}

bool SegmentAnalytics::DataStorage::parseFrom(const rapidjson::Value & value) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!value.IsObject()) {
		spdlog::error("Invalid Segment analytics data type: '{}', expected 'object'.", Utilities::typeToString(value.GetType()));
		return false;
	}

	// check for unhandled segment analytics data properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = value.MemberBegin(); i != value.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_SEGMENT_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Segment analytics data has unexpected property: '{}'.", i->name.GetString());
		}
	}

	if(value.HasMember(JSON_SEGMENT_FILE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & fileTypeValue = value[JSON_SEGMENT_FILE_TYPE_PROPERTY_NAME];

		if(!fileTypeValue.IsString()) {
			spdlog::error("Invalid Segment analytics data file type type: '{}', expected: 'string'.", Utilities::typeToString(fileTypeValue.GetType()));
			return false;
		}

		if(!Utilities::areStringsEqualIgnoreCase(fileTypeValue.GetString(), FILE_TYPE)) {
			spdlog::error("Incorrect Segment analytics data file type: '{}', expected: '{}'.", fileTypeValue.GetString(), FILE_TYPE);
			return false;
		}
	}
	else {
		spdlog::warn("Segment analytics JSON data is missing file type, and may fail to load correctly!");
	}

	if(value.HasMember(JSON_SEGMENT_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = value[JSON_SEGMENT_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid Segment analytics data file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid Segment analytics data file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported Segment analytics data file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Segment analytics JSON data is missing file format version, and may fail to load correctly!");
	}

	// parse previous session number property
	if(!value.HasMember(JSON_SEGMENT_PREVIOUS_SESSION_NUMBER_PROPERTY_NAME)) {
		spdlog::error("Segment analytics data is missing '{}' property.", JSON_SEGMENT_PREVIOUS_SESSION_NUMBER_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & previousSessionNumberValue = value[JSON_SEGMENT_PREVIOUS_SESSION_NUMBER_PROPERTY_NAME];

	if(!previousSessionNumberValue.IsUint64()) {
		spdlog::error("Segment analytics data has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_SEGMENT_PREVIOUS_SESSION_NUMBER_PROPERTY_NAME, Utilities::typeToString(previousSessionNumberValue.GetType()));
		return nullptr;
	}

	uint64_t previousSessionNumber = previousSessionNumberValue.GetUint64();

	if(previousSessionNumber == 0) {
		spdlog::error("Segment analytics data has an invalid '{}' property value: '{}', expected positive integer greater than zero.", JSON_SEGMENT_PREVIOUS_SESSION_NUMBER_PROPERTY_NAME, Utilities::typeToString(previousSessionNumberValue.GetType()));
		return nullptr;
	}

	// parse previous application version property
	if(!value.HasMember(JSON_SEGMENT_PREVIOUS_APPLICATION_VERSION_PROPERTY_NAME)) {
		spdlog::error("Segment analytics data is missing '{}' property.", JSON_SEGMENT_PREVIOUS_APPLICATION_VERSION_PROPERTY_NAME);
		return false;
	}

	const rapidjson::Value & previousApplicationVersionValue = value[JSON_SEGMENT_PREVIOUS_APPLICATION_VERSION_PROPERTY_NAME];

	if(!previousApplicationVersionValue.IsString()) {
		spdlog::error("Segment analytics data has an invalid '{}' property type: '{}', expected 'string'.", JSON_SEGMENT_PREVIOUS_APPLICATION_VERSION_PROPERTY_NAME, Utilities::typeToString(previousApplicationVersionValue.GetType()));
		return false;
	}

	std::string previousApplicationVersion(Utilities::trimString(previousApplicationVersionValue.GetString()));

	if(previousApplicationVersion.empty()) {
		spdlog::error("Segment analytics data '{}' property value cannot be empty.", JSON_SEGMENT_PREVIOUS_APPLICATION_VERSION_PROPERTY_NAME);
		return false;
	}

	// parse previous application build property
	if(!value.HasMember(JSON_SEGMENT_PREVIOUS_APPLICATION_BUILD_PROPERTY_NAME)) {
		spdlog::error("Segment analytics data is missing '{}' property.", JSON_SEGMENT_PREVIOUS_APPLICATION_BUILD_PROPERTY_NAME);
		return false;
	}

	const rapidjson::Value & previousApplicationBuildValue = value[JSON_SEGMENT_PREVIOUS_APPLICATION_BUILD_PROPERTY_NAME];

	if(!previousApplicationBuildValue.IsString()) {
		spdlog::error("Segment analytics data has an invalid '{}' property type: '{}', expected 'string'.", JSON_SEGMENT_PREVIOUS_APPLICATION_BUILD_PROPERTY_NAME, Utilities::typeToString(previousApplicationBuildValue.GetType()));
		return false;
	}

	std::string previousApplicationBuild(Utilities::trimString(previousApplicationBuildValue.GetString()));

	if(previousApplicationBuild.empty()) {
		spdlog::error("Segment analytics data '{}' property value cannot be empty.", JSON_SEGMENT_PREVIOUS_APPLICATION_BUILD_PROPERTY_NAME);
		return false;
	}

	// parse anonymous id property
	if(!value.HasMember(JSON_SEGMENT_ANONYMOUS_ID_PROPERTY_NAME)) {
		spdlog::error("Segment analytics data is missing '{}' property.", JSON_SEGMENT_ANONYMOUS_ID_PROPERTY_NAME);
		return false;
	}

	const rapidjson::Value & anonymousIDValue = value[JSON_SEGMENT_ANONYMOUS_ID_PROPERTY_NAME];

	if(!anonymousIDValue.IsString()) {
		spdlog::error("Segment analytics data has an invalid '{}' property type: '{}', expected 'string'.", JSON_SEGMENT_ANONYMOUS_ID_PROPERTY_NAME, Utilities::typeToString(anonymousIDValue.GetType()));
		return false;
	}

	std::string anonymousID(Utilities::trimString(anonymousIDValue.GetString()));

	if(anonymousID.empty()) {
		spdlog::error("Segment analytics data '{}' property value cannot be empty.", JSON_SEGMENT_ANONYMOUS_ID_PROPERTY_NAME);
		return false;
	}

	// parse analytic event id counter property
	std::optional<uint64_t> analyticEventIDCounter;

	if(value.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_ID_COUNTER_PROPERTY_NAME)) {
		const rapidjson::Value & analyticEventIDCounterValue = value[JSON_SEGMENT_ANALYTIC_EVENT_ID_COUNTER_PROPERTY_NAME];

		if(!analyticEventIDCounterValue.IsUint64()) {
			spdlog::error("Segment analytics data has an invalid '{}' property type: '{}', expected 'number'.", JSON_SEGMENT_ANALYTIC_EVENT_ID_COUNTER_PROPERTY_NAME, Utilities::typeToString(analyticEventIDCounterValue.GetType()));
			return false;
		}

		analyticEventIDCounter = analyticEventIDCounterValue.GetUint64();

		if(analyticEventIDCounter.value() == 0) {
			spdlog::error("Segment analytics data has an invalid '{}' property value: '{}', expected positive integer greater than zero.", JSON_SEGMENT_ANALYTIC_EVENT_ID_COUNTER_PROPERTY_NAME, analyticEventIDCounter.value());
			return false;
		}
	}

	// parse analytic events property
	std::map<uint64_t, std::shared_ptr<SegmentAnalyticEvent>> pendingAnalyticEvents;

	if(value.HasMember(JSON_SEGMENT_ANALYTIC_EVENTS_PROPERTY_NAME)) {
		const rapidjson::Value & analyticEventsValue = value[JSON_SEGMENT_ANALYTIC_EVENTS_PROPERTY_NAME];

		if(!analyticEventsValue.IsArray()) {
			spdlog::error("Segment analytics data has an invalid '{}' property type: '{}', expected 'array'.", JSON_SEGMENT_ANALYTIC_EVENTS_PROPERTY_NAME, Utilities::typeToString(analyticEventsValue.GetType()));
			return false;
		}

		uint64_t currentAnalyticEventID = 0;

		for(rapidjson::Value::ConstValueIterator i = analyticEventsValue.Begin(); i != analyticEventsValue.End(); ++i) {
			std::unique_ptr<SegmentAnalyticEvent> analyticEvent(SegmentAnalyticEvent::parseFrom(*i));

			if(!SegmentAnalyticEvent::isValid(analyticEvent.get())) {
				return false;
			}

			currentAnalyticEventID = analyticEvent->getID();
			pendingAnalyticEvents[currentAnalyticEventID] = std::move(analyticEvent);
		}
	}

	m_firstApplicationLaunch = false;
	m_sessionNumber = previousSessionNumber + 1;
	m_previousApplicationVersion = std::move(previousApplicationVersion);
	m_previousApplicationBuild = std::move(previousApplicationBuild);
	m_anonymousID = std::move(anonymousID);

	if(analyticEventIDCounter.has_value()) {
		SegmentAnalyticEvent::setIDCounter(analyticEventIDCounter.value());
	}

	m_pendingAnalyticEvents = std::move(pendingAnalyticEvents);

	return true;
}

bool SegmentAnalytics::DataStorage::load() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!std::filesystem::is_regular_file(std::filesystem::path(m_filePath))) {
		return false;
	}

	std::ifstream fileStream(m_filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document dataDocument;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(dataDocument.ParseStream(fileStreamWrapper).HasParseError()) {
		return false;
	}

	fileStream.close();

	return parseFrom(dataDocument);
}

bool SegmentAnalytics::DataStorage::save() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	std::ofstream fileStream(m_filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document dataDocument(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	dataDocument.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

bool SegmentAnalytics::DataStorage::createRequiredDirectories() {
	if(m_filePath.empty()) {
		spdlog::error("Missing Segment analytics data storage file path setting.");
		return false;
	}

	std::error_code errorCode;
	std::filesystem::path cacheDirectoryPath;
	size_t firstPathSeparatorIndex = m_filePath.find_first_of("/");

	if(firstPathSeparatorIndex != std::string::npos && firstPathSeparatorIndex != m_filePath.length() - 1) {
		cacheDirectoryPath = std::filesystem::path(Utilities::getFilePath(Utilities::trimTrailingPathSeparator(m_filePath)));
	}

	if(cacheDirectoryPath.empty()) {
		spdlog::error("Invalid Segment analytics data storage file path setting.");
		return false;
	}

	if(!std::filesystem::is_directory(cacheDirectoryPath)) {
		std::filesystem::create_directories(cacheDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create Segment analytics data storage file base directory structure '{}': {}", cacheDirectoryPath.string(), errorCode.message());
			return false;
		}

		spdlog::debug("Created Segment analytics data storage file base directory structure: '{}'.", cacheDirectoryPath.string());
	}

	return true;
}

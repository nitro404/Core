#include "SegmentAnalyticEvent.h"

#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"
#include "Utilities/Utilities.h"

#include <fmt/core.h>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <sstream>

static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_TYPE_PROPERTY_NAME = "type";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_CATEGORY_PROPERTY_NAME = "category";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_TIMESTAMP_PROPERTY_NAME = "timestamp";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_USER_ID_PROPERTY_NAME = "userID";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_PROPERTIES_PROPERTY_NAME = "properties";
static constexpr const char * JSON_SEGMENT_ANALYTIC_EVENT_USER_TRAITS_PROPERTY_NAME = "userTraits";
static const std::array<std::string_view, 8> JSON_SEGMENT_ANALYTIC_EVENT_PROPERTY_NAMES = {
	JSON_SEGMENT_ANALYTIC_EVENT_ID_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENT_TYPE_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENT_NAME_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENT_CATEGORY_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENT_TIMESTAMP_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENT_USER_ID_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENT_PROPERTIES_PROPERTY_NAME,
	JSON_SEGMENT_ANALYTIC_EVENT_USER_TRAITS_PROPERTY_NAME
};

uint64_t SegmentAnalyticEvent::s_idCounter = 1;

SegmentAnalyticEvent::SegmentAnalyticEvent(EventType type, const std::string & name, const std::string & category, const std::map<std::string, std::any> & properties, const std::string & userID, const std::map<std::string, std::any> & userTraits)
	: m_id(s_idCounter++)
	, m_type(type)
	, m_name(name)
	, m_category(SegmentAnalyticEvent::canHaveCategory(type) ? category : "")
	, m_timestamp(std::chrono::system_clock::now())
	, m_userID(SegmentAnalyticEvent::canHaveUserTraits(type) ? userID : "") {
	if(SegmentAnalyticEvent::canHaveProperties(m_type)) {
		for(std::map<std::string, std::any>::const_iterator i = properties.cbegin(); i != properties.cend(); ++i) {
			if(!isValidPropertyName(i->first)) {
				continue;
			}

			m_properties[i->first] = i->second;
		}
	}

	if(canHaveUserTraits(m_type)) {
		for(std::map<std::string, std::any>::const_iterator i = userTraits.cbegin(); i != userTraits.cend(); ++i) {
			if(!isValidPropertyName(i->first)) {
				continue;
			}

			m_userTraits[i->first] = i->second;
		}
	}
}

SegmentAnalyticEvent::SegmentAnalyticEvent(uint64_t id, EventType type, const std::string & name, const std::string & category, std::chrono::time_point<std::chrono::system_clock> timestamp, const std::map<std::string, std::any> & properties, const std::string & userID, const std::map<std::string, std::any> & userTraits)
	: m_id(id)
	, m_type(type)
	, m_name(name)
	, m_category(SegmentAnalyticEvent::canHaveCategory(type) ? category : "")
	, m_timestamp(timestamp)
	, m_userID(SegmentAnalyticEvent::canHaveUserTraits(type) ? userID : "") {
	if(SegmentAnalyticEvent::canHaveProperties(m_type)) {
		for(std::map<std::string, std::any>::const_iterator i = properties.cbegin(); i != properties.cend(); ++i) {
			if(!isValidPropertyName(i->first)) {
				continue;
			}

			m_properties[i->first] = i->second;
		}
	}

	if(canHaveUserTraits(m_type)) {
		for(std::map<std::string, std::any>::const_iterator i = userTraits.cbegin(); i != userTraits.cend(); ++i) {
			if(!isValidPropertyName(i->first)) {
				continue;
			}

			m_userTraits[i->first] = i->second;
		}
	}
}

SegmentAnalyticEvent::SegmentAnalyticEvent(SegmentAnalyticEvent && e) noexcept
	: m_id(e.m_id)
	, m_type(e.m_type)
	, m_name(std::move(e.m_name))
	, m_category(std::move(e.m_category))
	, m_timestamp(e.m_timestamp)
	, m_properties(std::move(e.m_properties)) { }

SegmentAnalyticEvent::SegmentAnalyticEvent(const SegmentAnalyticEvent & e)
	: m_id(s_idCounter++)
	, m_type(e.m_type)
	, m_name(e.m_name)
	, m_category(e.m_category)
	, m_timestamp(e.m_timestamp)
	, m_properties(e.m_properties) { }

SegmentAnalyticEvent & SegmentAnalyticEvent::operator = (SegmentAnalyticEvent && e) noexcept {
	if(this != &e) {
		m_id = e.m_id;
		m_type = e.m_type;
		m_name = std::move(e.m_name);
		m_category = std::move(e.m_category);
		m_timestamp = e.m_timestamp;
		m_properties = std::move(e.m_properties);
	}

	return *this;
}

SegmentAnalyticEvent & SegmentAnalyticEvent::operator = (const SegmentAnalyticEvent & e) {
	m_type = e.m_type;
	m_name = e.m_name;
	m_category = e.m_category;
	m_timestamp = e.m_timestamp;
	m_properties = e.m_properties;

	return *this;
}

SegmentAnalyticEvent::~SegmentAnalyticEvent() = default;

uint64_t SegmentAnalyticEvent::getID() const {
	return m_id;
}

uint64_t SegmentAnalyticEvent::getIDCounter() {
	return s_idCounter;
}

void SegmentAnalyticEvent::setIDCounter(uint64_t idCounter) {
	s_idCounter = idCounter;
}

SegmentAnalyticEvent::EventType SegmentAnalyticEvent::getType() const {
	return m_type;
}

const std::string & SegmentAnalyticEvent::getName() const {
	return m_name;
}

bool SegmentAnalyticEvent::hasCategory() {
	return !m_category.empty();
}

const std::string & SegmentAnalyticEvent::getCategory() const {
	return m_category;
}

std::chrono::time_point<std::chrono::system_clock> SegmentAnalyticEvent::getTimestamp() const {
	return m_timestamp;
}

bool SegmentAnalyticEvent::hasUserID() const {
	return !m_userID.empty();
}

const std::string & SegmentAnalyticEvent::getUserID() const {
	return m_userID;
}

bool SegmentAnalyticEvent::hasProperties() const {
	return !m_properties.empty();
}

size_t SegmentAnalyticEvent::numberOfProperties() const {
	return m_properties.size();
}

bool SegmentAnalyticEvent::hasProperty(const std::string & propertyName) const {
	return getProperty(propertyName) != nullptr;
}

const std::any * SegmentAnalyticEvent::getProperty(const std::string & propertyName) const {
	if(!isValidPropertyName(propertyName)) {
		return nullptr;
	}

	std::map<std::string, std::any>::const_iterator property(m_properties.find(propertyName));

	if(property == m_properties.end()) {
		return nullptr;
	}

	return &property->second;
}

const std::map<std::string, std::any> SegmentAnalyticEvent::getProperties() const {
	return m_properties;
}

bool SegmentAnalyticEvent::setProperty(const std::string & propertyName, const std::any & value) {
	if(!canHaveProperties(m_type) || !isValidPropertyName(propertyName)) {
		return false;
	}

	m_properties[propertyName] = value;

	return true;
}

bool SegmentAnalyticEvent::setProperties(const std::map<std::string, std::any> & properties) {
	if(!canHaveProperties(m_type)) {
		return false;
	}

	for(std::map<std::string, std::any>::const_iterator i = properties.begin(); i != properties.end(); ++i) {
		if(!isValidPropertyName(i->first)) {
			return false;
		}
	}

	for(std::map<std::string, std::any>::const_iterator i = properties.begin(); i != properties.end(); ++i) {
		m_properties[i->first] = i->second;
	}

	return true;
}

void SegmentAnalyticEvent::removeProperty(const std::string & propertyName) {
	if(!canHaveProperties(m_type) || !isValidPropertyName(propertyName)) {
		return;
	}

	m_properties.erase(propertyName);
}

void SegmentAnalyticEvent::clearProperties() {
	m_properties.clear();
}

bool SegmentAnalyticEvent::hasUserTraits() const {
	return !m_userTraits.empty();
}

size_t SegmentAnalyticEvent::numberOfUserTraits() const {
	return m_userTraits.size();
}

bool SegmentAnalyticEvent::hasUserTrait(const std::string & userTraitName) const {
	return getUserTrait(userTraitName) != nullptr;
}

const std::any * SegmentAnalyticEvent::getUserTrait(const std::string userTraitName) const {
	if(!isValidPropertyName(userTraitName)) {
		return nullptr;
	}

	std::map<std::string, std::any>::const_iterator userTrait(m_userTraits.find(userTraitName));

	if(userTrait == m_userTraits.end()) {
		return nullptr;
	}

	return &userTrait->second;
}

const std::map<std::string, std::any> & SegmentAnalyticEvent::getUserTraits() const {
	return m_userTraits;
}

bool SegmentAnalyticEvent::setUserTrait(const std::string & userTraitName, const std::any & value) {
	if(!canHaveUserTraits(m_type) || !isValidPropertyName(userTraitName)) {
		return false;
	}

	m_userTraits[userTraitName] = value;

	return true;
}

bool SegmentAnalyticEvent::setUserTraits(const std::map<std::string, std::any> & userTraits) {
	if(!canHaveUserTraits(m_type)) {
		return false;
	}

	for(std::map<std::string, std::any>::const_iterator i = userTraits.begin(); i != userTraits.end(); ++i) {
		if(!isValidPropertyName(i->first)) {
			return false;
		}
	}

	for(std::map<std::string, std::any>::const_iterator i = userTraits.begin(); i != userTraits.end(); ++i) {
		m_userTraits[i->first] = i->second;
	}

	return true;
}

void SegmentAnalyticEvent::removeUserTrait(const std::string & userTraitName) {
	if(!canHaveUserTraits(m_type) || !isValidPropertyName(userTraitName)) {
		return;
	}

	m_userTraits.erase(userTraitName);
}

void SegmentAnalyticEvent::clearUserTraits() {
	m_userTraits.clear();
}

std::string SegmentAnalyticEvent::toDebugString() const {
	std::stringstream debugStringStream;

	debugStringStream << fmt::format("Type: {}\n", Utilities::toCapitalCase(magic_enum::enum_name(m_type)));
	debugStringStream << fmt::format("Name: '{}'\n", m_name);

	if(!m_category.empty()) {
		debugStringStream << fmt::format("Category: '{}'\n", m_category);
	}

	debugStringStream << fmt::format("Timestamp: '{}'\n", Utilities::timePointToString(m_timestamp));

	if(!m_userID.empty()) {
		debugStringStream << fmt::format("User ID: '{}'\n", m_userID);
	}

	if(!m_properties.empty()) {
		debugStringStream << fmt::format("Properties ({}):\n", m_properties.size());

		for(std::map<std::string, std::any>::const_iterator i = m_properties.cbegin(); i != m_properties.cend(); ++i) {
			debugStringStream << fmt::format(" - {}: '{}'\n", i->first, Utilities::anyToString(i->second));
		}
	}

	if(!m_userTraits.empty()) {
		debugStringStream << fmt::format("User Traits ({}):\n", m_userTraits.size());

		for(std::map<std::string, std::any>::const_iterator i = m_userTraits.cbegin(); i != m_userTraits.cend(); ++i) {
			debugStringStream << fmt::format(" - {}: '{}'\n", i->first, Utilities::anyToString(i->second));
		}
	}

	return debugStringStream.str();
}

rapidjson::Value SegmentAnalyticEvent::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value analyticEventValue(rapidjson::kObjectType);

	analyticEventValue.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_ID_PROPERTY_NAME), rapidjson::Value(m_id), allocator);

	rapidjson::Value analyticEventTypeValue(magic_enum::enum_name(m_type).data(), allocator);
	analyticEventValue.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_TYPE_PROPERTY_NAME), analyticEventTypeValue, allocator);

	rapidjson::Value analyticEventNameValue(m_name.c_str(), allocator);
	analyticEventValue.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_NAME_PROPERTY_NAME), analyticEventNameValue, allocator);

	if(!m_category.empty()) {
		rapidjson::Value analyticEventCategoryValue(m_category.c_str(), allocator);
		analyticEventValue.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_CATEGORY_PROPERTY_NAME), analyticEventCategoryValue, allocator);
	}

	analyticEventValue.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_TIMESTAMP_PROPERTY_NAME), rapidjson::Value(std::chrono::time_point_cast<std::chrono::milliseconds>(m_timestamp).time_since_epoch().count()), allocator);

	if(!m_userID.empty()) {
		rapidjson::Value userIDValue(m_userID.c_str(), allocator);
		analyticEventValue.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_USER_ID_PROPERTY_NAME), userIDValue, allocator);
	}

	if(!m_properties.empty()) {
		rapidjson::Value propertiesValue(Utilities::anyMapToJSONValue(m_properties, allocator, true));
		analyticEventValue.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_PROPERTIES_PROPERTY_NAME), propertiesValue, allocator);
	}

	if(!m_userTraits.empty()) {
		rapidjson::Value userTraitsValue(Utilities::anyMapToJSONValue(m_userTraits, allocator, true));
		analyticEventValue.AddMember(rapidjson::StringRef(JSON_SEGMENT_ANALYTIC_EVENT_USER_TRAITS_PROPERTY_NAME), userTraitsValue, allocator);
	}
	return analyticEventValue;
}

std::unique_ptr<SegmentAnalyticEvent> SegmentAnalyticEvent::parseFrom(const rapidjson::Value & analyticEventValue) {
	if(!analyticEventValue.IsObject()) {
		spdlog::error("Invalid Segment analytic event type: '{}', expected 'object'.", Utilities::typeToString(analyticEventValue.GetType()));
		return nullptr;
	}

	// check for unhandled segment analytics data properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = analyticEventValue.MemberBegin(); i != analyticEventValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_SEGMENT_ANALYTIC_EVENT_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Segment analytic event has unexpected property: '{}'.", i->name.GetString());
		}
	}

	// parse analytic event ID
	if(!analyticEventValue.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_ID_PROPERTY_NAME)) {
		spdlog::error("Segment analytic event is missing '{}' property.", JSON_SEGMENT_ANALYTIC_EVENT_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & idValue = analyticEventValue[JSON_SEGMENT_ANALYTIC_EVENT_ID_PROPERTY_NAME];

	if(!idValue.IsUint64()) {
		spdlog::error("Segment analytic event has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_SEGMENT_ANALYTIC_EVENT_ID_PROPERTY_NAME, Utilities::typeToString(idValue.GetType()));
		return nullptr;
	}

	uint64_t id = idValue.GetUint64();

	if(id == 0) {
		spdlog::error("Segment analytic event has an invalid '{}' property value: '{}', expected positive integer greater than zero.", JSON_SEGMENT_ANALYTIC_EVENT_ID_PROPERTY_NAME, Utilities::typeToString(idValue.GetType()));
		return nullptr;
	}

	// parse analytic event type
	if(!analyticEventValue.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_TYPE_PROPERTY_NAME)) {
		spdlog::error("Segment analytic event is missing '{}' property.", JSON_SEGMENT_ANALYTIC_EVENT_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & typeValue = analyticEventValue[JSON_SEGMENT_ANALYTIC_EVENT_TYPE_PROPERTY_NAME];

	if(!typeValue.IsString()) {
		spdlog::error("Segment analytic event has an invalid '{}' property type: '{}', expected 'string'.", JSON_SEGMENT_ANALYTIC_EVENT_TYPE_PROPERTY_NAME, Utilities::typeToString(typeValue.GetType()));
		return nullptr;
	}

	std::optional<SegmentAnalyticEvent::EventType> optionalType(magic_enum::enum_cast<SegmentAnalyticEvent::EventType>(typeValue.GetString()));

	if(!optionalType.has_value()) {
		spdlog::error("Segment analytic event has an invalid '{}' property value: '{}'.", JSON_SEGMENT_ANALYTIC_EVENT_TYPE_PROPERTY_NAME, typeValue.GetString());
		return nullptr;
	}

	// parse analytic event name
	if(!analyticEventValue.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_NAME_PROPERTY_NAME)) {
		spdlog::error("Segment analytic event is missing '{}' property.", JSON_SEGMENT_ANALYTIC_EVENT_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & nameValue = analyticEventValue[JSON_SEGMENT_ANALYTIC_EVENT_NAME_PROPERTY_NAME];

	if(!nameValue.IsString()) {
		spdlog::error("Segment analytic event has an invalid '{}' property type: '{}', expected 'string'.", JSON_SEGMENT_ANALYTIC_EVENT_NAME_PROPERTY_NAME, Utilities::typeToString(nameValue.GetType()));
		return nullptr;
	}

	std::string name(Utilities::trimString(nameValue.GetString()));

	if(name.empty()) {
		spdlog::error("Segment analytic event '{}' property cannot be empty.", JSON_SEGMENT_ANALYTIC_EVENT_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse analytic event category
	std::string category;

	if(analyticEventValue.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_CATEGORY_PROPERTY_NAME)) {
		const rapidjson::Value & categoryValue = analyticEventValue[JSON_SEGMENT_ANALYTIC_EVENT_CATEGORY_PROPERTY_NAME];

		if(!categoryValue.IsString()) {
			spdlog::error("Segment analytic event has an invalid '{}' property type: '{}', expected 'string'.", JSON_SEGMENT_ANALYTIC_EVENT_CATEGORY_PROPERTY_NAME, Utilities::typeToString(categoryValue.GetType()));
			return nullptr;
		}

		category = Utilities::trimString(categoryValue.GetString());
	}

	// parse analytic event timestamp
	if(!analyticEventValue.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_TIMESTAMP_PROPERTY_NAME)) {
		spdlog::error("Segment analytic event is missing '{}' property.", JSON_SEGMENT_ANALYTIC_EVENT_TIMESTAMP_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & timestampValue = analyticEventValue[JSON_SEGMENT_ANALYTIC_EVENT_TIMESTAMP_PROPERTY_NAME];

	if(!timestampValue.IsUint64()) {
		spdlog::error("Segment analytic event has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_SEGMENT_ANALYTIC_EVENT_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(timestampValue.GetType()));
		return nullptr;
	}

	std::chrono::time_point<std::chrono::system_clock> timestamp = std::chrono::system_clock::from_time_t(time_t{0}) + std::chrono::milliseconds(timestampValue.GetUint64());

	// parse analytic event user ID
	std::string userID;

	if(analyticEventValue.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_USER_ID_PROPERTY_NAME)) {
		const rapidjson::Value & userIDValue = analyticEventValue[JSON_SEGMENT_ANALYTIC_EVENT_USER_ID_PROPERTY_NAME];

		if(!userIDValue.IsString()) {
			spdlog::error("Segment analytic event has an invalid '{}' property type: '{}', expected 'string'.", JSON_SEGMENT_ANALYTIC_EVENT_USER_ID_PROPERTY_NAME, Utilities::typeToString(userIDValue.GetType()));
			return nullptr;
		}

		userID = Utilities::trimString(userIDValue.GetString());
	}

	// parse analytic event properties
	std::map<std::string, std::any> properties;

	if(analyticEventValue.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_PROPERTIES_PROPERTY_NAME)) {
		const rapidjson::Value & propertiesValue = analyticEventValue[JSON_SEGMENT_ANALYTIC_EVENT_PROPERTIES_PROPERTY_NAME];

		properties = Utilities::jsonObjectToAnyMap(propertiesValue);
	}

	// parse analytic event user traits
	std::map<std::string, std::any> userTraits;

	if(analyticEventValue.HasMember(JSON_SEGMENT_ANALYTIC_EVENT_USER_TRAITS_PROPERTY_NAME)) {
		const rapidjson::Value & userTraitsValue = analyticEventValue[JSON_SEGMENT_ANALYTIC_EVENT_USER_TRAITS_PROPERTY_NAME];

		userTraits = Utilities::jsonObjectToAnyMap(userTraitsValue);
	}

	return std::unique_ptr<SegmentAnalyticEvent>(new SegmentAnalyticEvent(id, optionalType.value(), name, category, timestamp, properties, userID, userTraits));
}

std::unique_ptr<SegmentAnalyticEvent> SegmentAnalyticEvent::createIdentifyEvent(const std::string & userID, const std::map<std::string, std::any> & traits) {
	return std::unique_ptr<SegmentAnalyticEvent>(new SegmentAnalyticEvent(SegmentAnalyticEvent::EventType::Identify, Utilities::emptyString, Utilities::emptyString, {}, userID, traits));
}

std::unique_ptr<SegmentAnalyticEvent> SegmentAnalyticEvent::createAliasEvent(const std::string & previousUserID) {
	return std::unique_ptr<SegmentAnalyticEvent>(new SegmentAnalyticEvent(SegmentAnalyticEvent::EventType::Alias, previousUserID, Utilities::emptyString, {}, Utilities::emptyString, {}));
}

std::unique_ptr<SegmentAnalyticEvent> SegmentAnalyticEvent::createGroupEvent(const std::string & group, const std::map<std::string, std::any> & traits) {
	return std::unique_ptr<SegmentAnalyticEvent>(new SegmentAnalyticEvent(SegmentAnalyticEvent::EventType::Group, group, Utilities::emptyString, {}, Utilities::emptyString, traits));
}

std::unique_ptr<SegmentAnalyticEvent> SegmentAnalyticEvent::createTrackEvent(const std::string & name, const std::map<std::string, std::any> & properties, const std::string & userID, const std::map<std::string, std::any> & userTraits) {
	return std::unique_ptr<SegmentAnalyticEvent>(new SegmentAnalyticEvent(SegmentAnalyticEvent::EventType::Track, name, Utilities::emptyString, properties, userID, userTraits));
}

std::unique_ptr<SegmentAnalyticEvent> SegmentAnalyticEvent::createScreenEvent(const std::string & name, const std::string & category, const std::map<std::string, std::any> & properties, const std::string & userID, const std::map<std::string, std::any> & userTraits) {
	return std::unique_ptr<SegmentAnalyticEvent>(new SegmentAnalyticEvent(SegmentAnalyticEvent::EventType::Screen, name, category, properties, userID, userTraits));
}

bool SegmentAnalyticEvent::isValid() const {
	if(!canHaveProperties(m_type) && !m_properties.empty()) {
		return false;
	}

	if(!canHaveCategory(m_type) && !m_category.empty()) {
		return false;
	}

	return !m_name.empty();
}

bool SegmentAnalyticEvent::isValid(const SegmentAnalyticEvent * e) {
	return e != nullptr && e->isValid();
}

bool SegmentAnalyticEvent::isValidPropertyName(const std::string & propertyName) {
	return !propertyName.empty() && propertyName.find_first_of(" \t") == std::string::npos;
}

bool SegmentAnalyticEvent::canHaveCategory(EventType type) {
	return type == EventType::Screen;
}

bool SegmentAnalyticEvent::canHaveProperties(EventType type) {
	switch(type) {
		case EventType::Alias:
		case EventType::Identify:
		case EventType::Group:
			return false;
		case EventType::Track:
		case EventType::Screen:
			return true;
	}

	return false;
}

bool SegmentAnalyticEvent::canHaveUserTraits(EventType type) {
	switch(type) {
		case EventType::Alias:
			return false;
		case EventType::Identify:
		case EventType::Group:
		case EventType::Track:
		case EventType::Screen:
			return true;
	}

	return false;
}

bool SegmentAnalyticEvent::operator == (const SegmentAnalyticEvent & e) const {
	return m_id == e.m_id;
}

bool SegmentAnalyticEvent::operator != (const SegmentAnalyticEvent & e) const {
	return !operator == (e);
}

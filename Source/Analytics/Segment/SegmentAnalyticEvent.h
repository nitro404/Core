#ifndef _SEGMENT_ANALYTIC_EVENT_H_
#define _SEGMENT_ANALYTIC_EVENT_H_

#include <rapidjson/document.h>

#include <any>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

class SegmentAnalyticEvent final {
public:
	enum class EventType {
		Identify,
		Alias,
		Group,
		Track,
		Screen
	};

	SegmentAnalyticEvent(SegmentAnalyticEvent && e) noexcept;
	SegmentAnalyticEvent(const SegmentAnalyticEvent & e);
	SegmentAnalyticEvent & operator = (SegmentAnalyticEvent && e) noexcept;
	SegmentAnalyticEvent & operator = (const SegmentAnalyticEvent & e);
	~SegmentAnalyticEvent();

	uint64_t getID() const;
	static uint64_t getIDCounter();
	static void setIDCounter(uint64_t idCounter);
	EventType getType() const;
	const std::string & getName() const;
	bool hasCategory();
	const std::string & getCategory() const;
	std::chrono::time_point<std::chrono::system_clock> getTimestamp() const;
	bool hasUserID() const;
	const std::string & getUserID() const;
	bool hasProperties() const;
	size_t numberOfProperties() const;
	bool hasProperty(const std::string & propertyName) const;
	const std::any * getProperty(const std::string & propertyName) const;
	const std::map<std::string, std::any> getProperties() const;
	bool setProperty(const std::string & propertyName, const std::any & value);
	bool setProperties(const std::map<std::string, std::any> & properties);
	void removeProperty(const std::string & propertyName);
	void clearProperties();
	bool hasUserTraits() const;
	size_t numberOfUserTraits() const;
	bool hasUserTrait(const std::string & userTraitName) const;
	const std::any * getUserTrait(const std::string userTraitName) const;
	const std::map<std::string, std::any> & getUserTraits() const;
	bool setUserTrait(const std::string & userTraitName, const std::any & value);
	bool setUserTraits(const std::map<std::string, std::any> & userTraits);
	void removeUserTrait(const std::string & userTraitName);
	void clearUserTraits();
	std::string toDebugString() const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<SegmentAnalyticEvent> parseFrom(const rapidjson::Value & analyticEventValue);

	static std::unique_ptr<SegmentAnalyticEvent> createIdentifyEvent(const std::string & userID, const std::map<std::string, std::any> & traits);
	static std::unique_ptr<SegmentAnalyticEvent> createAliasEvent(const std::string & previousUserID);
	static std::unique_ptr<SegmentAnalyticEvent> createGroupEvent(const std::string & group, const std::map<std::string, std::any> & traits);
	static std::unique_ptr<SegmentAnalyticEvent> createTrackEvent(const std::string & name, const std::map<std::string, std::any> & properties, const std::string & userID, const std::map<std::string, std::any> & userTraits);
	static std::unique_ptr<SegmentAnalyticEvent> createScreenEvent(const std::string & name, const std::string & category, const std::map<std::string, std::any> & properties, const std::string & userID, const std::map<std::string, std::any> & userTraits);

	bool isValid() const;
	static bool isValid(const SegmentAnalyticEvent * e);
	static bool isValidPropertyName(const std::string & propertyName);
	static bool canHaveCategory(EventType type);
	static bool canHaveProperties(EventType type);
	static bool canHaveUserTraits(EventType type);

	bool operator == (const SegmentAnalyticEvent & e) const;
	bool operator != (const SegmentAnalyticEvent & e) const;

private:
	SegmentAnalyticEvent(uint64_t id, EventType type, const std::string & name, const std::string & category, std::chrono::time_point<std::chrono::system_clock> m_timestamp, const std::map<std::string, std::any> & properties, const std::string & userID, const std::map<std::string, std::any> & userTraits);
	SegmentAnalyticEvent(EventType type, const std::string & name, const std::string & category, const std::map<std::string, std::any> & properties, const std::string & userID, const std::map<std::string, std::any> & userTraits);

	uint64_t m_id;
	EventType m_type;
	std::string m_name;
	std::string m_category;
	std::chrono::time_point<std::chrono::system_clock> m_timestamp;
	std::map<std::string, std::any> m_properties;
	std::string m_userID;
	std::map<std::string, std::any> m_userTraits;
	static uint64_t s_idCounter;
};

#endif // _SEGMENT_ANALYTIC_EVENT_H_

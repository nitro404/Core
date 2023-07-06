#ifndef _SEGMENT_ANALYTICS_H_
#define _SEGMENT_ANALYTICS_H_

#include "Dimension.h"
#include "Location/GeoLocation.h"
#include "SegmentAnalyticEvent.h"
#include "Singleton/Singleton.h"

#include <rapidjson/document.h>

#include <any>
#include <chrono>
#include <cstdint>
#include <condition_variable>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

class SegmentAnalytics : public Singleton<SegmentAnalytics> {
public:
	struct Configuration {
		virtual ~Configuration();

		std::string writeKey;
		bool includeIPAddress = false;
		bool includeGeoLocation = true;
		bool batchMode = true;
		uint16_t maxEventQueueSize = 20;
		std::chrono::milliseconds failedNetworkTransferRetryDelay = std::chrono::seconds(60);
		std::string dataStorageFilePath;
		std::string applicationName;
		std::string applicationVersion;
		std::string applicationBuild;
		std::string applicationPackageName;
		std::string userAgent;
	};

	virtual ~SegmentAnalytics();

	bool isInitialized() const;
	virtual bool initialize(const Configuration & configuration);
	bool isStarted() const;
	virtual bool start();
	virtual void stop();
	bool isFirstApplicationLaunch() const;
	uint64_t getSessionNumber() const;
	bool wasApplicationUpdated() const;
	std::string getPreviousApplicationVersion() const;
	std::string getPreviousApplicationBuild() const;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getSessionStartedTimePoint() const;
	std::optional<std::chrono::milliseconds> getSessionDuration() const;
	bool shouldIncludeIPAddress() const;
	const std::string & getIPAddress() const;
	bool shouldIncludeGeoLocation() const;
	const std::optional<GeoLocation> & getGeoLocation() const;
	bool isUsingBatchMode() const;
	uint16_t getMaxEventQueueSize() const;
	virtual bool shouldTrackApplicationEvents() const = 0;
	std::string getAnonymousID() const;
	bool setIdentity(const std::string & userID, const std::map<std::string, std::any> & traits = {});
	bool setAlias(const std::string & previousUserID);
	bool setGroup(const std::string & group, const std::map<std::string, std::any> & traits = {});
	bool track(const std::string & name, const std::map<std::string, std::any> & metrics = {});
	bool screen(const std::string & name, const std::map<std::string, std::any> & metrics = {});
	bool screen(const std::string & name, const std::string & category, const std::map<std::string, std::any> & metrics = {});
	virtual bool flush(std::chrono::milliseconds waitForDuration = std::chrono::milliseconds(0)) = 0;
	virtual void reset();
	bool onApplicationClosed();

protected:
	class LibraryInfoProvider {
	public:
		virtual ~LibraryInfoProvider();

		virtual const std::string & getLibraryName() const = 0;
		virtual const std::string & getLibraryVersion() const = 0;
	};

	class DataStorage final {
	public:
		DataStorage();
		DataStorage(DataStorage && dataStorage) noexcept;
		const DataStorage & operator = (DataStorage && dataStorage) noexcept;
		~DataStorage();

		bool isInitialized() const;
		bool initialize(const std::string & filePath, const std::string & applicationVersion, const std::string applicationBuild);
		std::string getFilePath() const;
		bool isFirstApplicationLaunch() const;
		uint64_t getSessionNumber() const;
		bool wasApplicationUpdated() const;
		std::string getPreviousApplicationVersion() const;
		std::string getPreviousApplicationBuild() const;
		std::string getAnonymousID() const;
		void setAnonymousID(const std::string & anonymousID);
		bool hasUserID() const;
		bool hasAnyUserTraits() const;
		std::string getUserID() const;
		const std::map<std::string, std::any> & getUserTraits() const;
		void setUserID(const std::string & userID);
		void setUserTraits(const std::map<std::string, std::any> & traits);
		void setUserData(const std::string & userID, const std::map<std::string, std::any> & traits);
		void clearUserData();
		bool hasAnyPendingAnalyticEvents() const;
		size_t numberOfPendingAnalyticEvents() const;
		bool hasPendingAnalyticEvent(const SegmentAnalyticEvent & analyticEvent) const;
		bool hasPendingAnalyticEventWithID(uint64_t analyticEventID) const;
		std::shared_ptr<SegmentAnalyticEvent> getPendingAnalyticEventWithID(uint64_t analyticEventID) const;
		std::vector<std::shared_ptr<SegmentAnalyticEvent>> getPendingAnalyticEvents() const;
		bool addPendingAnalyticEvent(std::shared_ptr<SegmentAnalyticEvent> analyticEvent);
		bool removePendingAnalyticEvent(const SegmentAnalyticEvent & analyticEvent);
		bool removePendingAnalyticEventWithID(uint64_t analyticEventID);
		size_t removePendingAnalyticEvents(const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents);
		bool clearPendingAnalyticEvents();
		void reset();

		static const std::string FILE_TYPE;
		static const std::string FILE_FORMAT_VERSION;

	private:
		rapidjson::Document toJSON() const;
		bool parseFrom(const rapidjson::Value & value);
		bool load();
		bool save() const;
		bool createRequiredDirectories();

		bool m_initialized;
		std::string m_filePath;
		bool m_firstApplicationLaunch;
		uint64_t m_sessionNumber;
		std::string m_previousApplicationVersion;
		std::string m_previousApplicationBuild;
		std::string m_anonymousID;
		std::string m_userID;
		std::string m_applicationVersion;
		std::string m_applicationBuild;
		std::map<std::string, std::any> m_userTraits;
		std::map<uint64_t, std::shared_ptr<SegmentAnalyticEvent>> m_pendingAnalyticEvents;
		mutable std::recursive_mutex m_mutex;

		DataStorage(const DataStorage &) = delete;
		const DataStorage & operator = (const DataStorage &) = delete;
	};

	SegmentAnalytics();

	const std::string & getWriteKey() const;
	const DataStorage * getDataStorage() const;
	DataStorage * getDataStorage();
	const std::string & getApplicationName() const;
	const std::string & getApplicationVersion() const;
	const std::string & getApplicationBuild() const;
	const std::string & getApplicationPackageName() const;
	const std::string & getUserAgent() const;
	bool onApplicationInstalled();
	bool onApplicationUpdated();
	bool onApplicationOpened();

	virtual const LibraryInfoProvider * getLibraryInfoProvider() const = 0;

	bool isConfigurationValid(const Configuration & configuration) const;
	virtual bool queueEvent(std::unique_ptr<SegmentAnalyticEvent> analyticEvent) = 0;

	std::unique_ptr<rapidjson::Document> createBaseEventPayloadDocument();
	static bool addEventDataToValue(const SegmentAnalyticEvent & analyticEvent, const std::string & anonymousID, rapidjson::Value & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool batchMode = false);

	mutable std::recursive_mutex m_mutex;
	mutable std::condition_variable_any m_waitCondition;

private:
	bool m_initialized;
	bool m_started;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_sessionSystemStartTimePoint;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> m_sessionSteadyStartTimePoint;
	std::string m_writeKey;
	bool m_includeIPAddress;
	std::string m_ipAddress;
	bool m_includeGeoLocation;
	std::optional<GeoLocation> m_geoLocation;
	bool m_batchMode;
	uint16_t m_maxEventQueueSize;
	std::string m_applicationName;
	std::string m_applicationVersion;
	std::string m_applicationBuild;
	std::string m_applicationPackageName;
	std::string m_userAgent;
	std::string m_anonymousID;
	std::string m_userID;
	std::unique_ptr<DataStorage> m_dataStorage;

	SegmentAnalytics(const SegmentAnalytics &) = delete;
	const SegmentAnalytics & operator = (const SegmentAnalytics &) = delete;
};

#endif // _SEGMENT_ANALYTICS_H_

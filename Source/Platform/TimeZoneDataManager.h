#ifndef _TIME_ZONE_DATA_MANAGER_H_
#define _TIME_ZONE_DATA_MANAGER_H_

#include <Singleton/Singleton.h>

#include <map>
#include <string>

class TimeZoneDataManager : public Singleton<TimeZoneDataManager> {
public:
	TimeZoneDataManager();
	virtual ~TimeZoneDataManager();

	bool isInitialized() const;
	bool initialize(const std::string & dataDirectoryPath, std::map<std::string, std::string> & fileETags, bool forceUpdate = false);

protected:
	virtual bool platformInitialize(const std::string & dataDirectoryPath, std::map<std::string, std::string> & fileETags, bool forceUpdate = false);

private:
	static std::string getCurrentTimeZoneDatabaseVersion(const std::string & dataDirectoryPath);
	static bool doAllTimeZoneDatabaseFilesExist(const std::string & dataDirectoryPath);
	static std::string getLatestTimeZoneDatabaseVersion();
	static std::string getTimeZoneDatabaseDownloadURL(const std::string & timeZoneDatabaseVersion);
	static std::string getLatestTimeZoneDatabaseDownloadURL();
	static bool updateTimeZoneDatabase(const std::string & dataDirectoryPath, std::map<std::string, std::string> & fileETags, bool forceUpdate = false);

	bool m_initialized;

	TimeZoneDataManager(const TimeZoneDataManager &) = delete;
	TimeZoneDataManager(TimeZoneDataManager &&) noexcept = delete;
	const TimeZoneDataManager & operator = (const TimeZoneDataManager &) = delete;
	const TimeZoneDataManager & operator = (TimeZoneDataManager &&) noexcept = delete;
};

#endif // _TIME_ZONE_DATA_MANAGER_H_

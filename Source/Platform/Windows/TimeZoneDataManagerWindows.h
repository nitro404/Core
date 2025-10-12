#ifndef _TIME_ZONE_DATA_MANAGER_WINDOWS_H_
#define _TIME_ZONE_DATA_MANAGER_WINDOWS_H_

#include "Platform/TimeZoneDataManager.h"

class TimeZoneDataManagerWindows final : public TimeZoneDataManager {
public:
	TimeZoneDataManagerWindows();
	virtual ~TimeZoneDataManagerWindows();

	virtual bool isSupported() const override;
	virtual bool platformInitialize(const std::string & dataDirectoryPath, std::map<std::string, std::string> & fileETags, bool shouldUpdate = true, bool forceUpdate = false, bool * updated = nullptr) override;

private:
	TimeZoneDataManagerWindows(const TimeZoneDataManagerWindows &) = delete;
	const TimeZoneDataManagerWindows & operator = (const TimeZoneDataManagerWindows &) = delete;
};

#endif // _TIME_ZONE_DATA_MANAGER_WINDOWS_H_

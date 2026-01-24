#ifndef _GEO_LOCATION_SERVICE_H_
#define _GEO_LOCATION_SERVICE_H_

#include "GeoLocation.h"
#include "Singleton/Singleton.h"

#include <memory>
#include <optional>
#include <string>

class GeoLocationService : public Singleton<GeoLocationService> {
public:
	~GeoLocationService() override;

	bool isInitialized() const;
	virtual bool initialize(const std::string & apiKey);
	virtual std::optional<GeoLocation> getGeoLocation() = 0;

protected:
	GeoLocationService();

	const std::string & getAPIKey() const;

private:
	bool m_initialized;
	std::string m_apiKey;
};

#endif // _GEO_LOCATION_SERVICE_H_

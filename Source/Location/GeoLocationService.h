#ifndef _GEO_LOCATION_SERVICE_H_
#define _GEO_LOCATION_SERVICE_H_

#include "GeoLocation.h"
#include "Singleton/Singleton.h"

#include <memory>
#include <optional>
#include <string>

class HTTPService;

class GeoLocationService : public Singleton<GeoLocationService> {
public:
	virtual ~GeoLocationService();

	bool isInitialized() const;
	virtual bool initialize(std::shared_ptr<HTTPService> httpService, const std::string & apiKey);
	virtual std::optional<GeoLocation> getGeoLocation() = 0;

protected:
	GeoLocationService();

	const std::string & getAPIKey() const;
	std::shared_ptr<HTTPService> getHTTPService() const;

private:
	bool m_initialized;
	std::shared_ptr<HTTPService> m_httpService;
	std::string m_apiKey;
};

#endif // _GEO_LOCATION_SERVICE_H_

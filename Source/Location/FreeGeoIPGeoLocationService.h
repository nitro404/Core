#ifndef _FREE_GEO_IP_GEO_LOCATION_SERVICE_H_
#define _FREE_GEO_IP_GEO_LOCATION_SERVICE_H_

#include "GeoLocationService.h"

#include <memory>

class HTTPService;

class FreeGeoIPGeoLocationService final : public GeoLocationService {
	friend class FactoryRegistry;

public:
	~FreeGeoIPGeoLocationService() override;

	virtual std::optional<GeoLocation> getGeoLocation() override;

private:
	FreeGeoIPGeoLocationService();
};

#endif // _FREE_GEO_IP_GEO_LOCATION_SERVICE_H_

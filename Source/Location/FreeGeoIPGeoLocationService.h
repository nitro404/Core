#ifndef _FREE_GEO_IP_GEO_LOCATION_SERVICE_H_
#define _FREE_GEO_IP_GEO_LOCATION_SERVICE_H_

#include "GeoLocationService.h"

#include <memory>

class HTTPService;

class FreeGeoIPGeoLocationService final : public GeoLocationService {
public:
	FreeGeoIPGeoLocationService();
	~FreeGeoIPGeoLocationService() override;

	virtual std::optional<GeoLocation> getGeoLocation() override;
};

#endif // _FREE_GEO_IP_GEO_LOCATION_SERVICE_H_

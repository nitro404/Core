#ifndef _GEO_LOCATION_H_
#define _GEO_LOCATION_H_

#include <string>

struct GeoLocation {
	std::string ipAddress;
	std::string countryName;
	std::string countryCode;
	std::string regionName;
	std::string regionCode;
	std::string cityName;
	std::string timeZone;
	double latitude;
	double longitude;
};

#endif // _GEO_LOCATION_H_

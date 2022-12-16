#include "FreeGeoIPGeoLocationService.h"

#include "Network/HTTPService.h"
#include "Utilities/RapidJSONUtilities.h"

#include <rapidjson/document.h>
#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

static const std::string FREE_GEO_IP_API_ADDRESS("https://api.freegeoip.app/json/");
static const std::string API_KEY_QUERY_PARAMETER("apikey");

static constexpr const char * JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME = "ip";
static constexpr const char * JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME = "country_code";
static constexpr const char * JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME = "country_name";
static constexpr const char * JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME = "region_code";
static constexpr const char * JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME = "region_name";
static constexpr const char * JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME = "city";
static constexpr const char * JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME = "time_zone";
static constexpr const char * JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME = "latitude";
static constexpr const char * JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME = "longitude";

FreeGeoIPGeoLocationService::FreeGeoIPGeoLocationService()
	: GeoLocationService() { }

FreeGeoIPGeoLocationService::~FreeGeoIPGeoLocationService() { }

std::optional<GeoLocation> FreeGeoIPGeoLocationService::getGeoLocation() {
	if(!isInitialized()) {
		return {};
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::error("Failed to retrieve geo location, HTTP service is not initialized!");
		return {};
	}

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, FREE_GEO_IP_API_ADDRESS + "?" + API_KEY_QUERY_PARAMETER + "=" + getAPIKey()));
	request->setConnectionTimeout(3s);
	request->setNetworkTimeout(2s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create Free GeoIP HTTP request!");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve geo location with error: {}", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to retrieve geo location ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::unique_ptr<rapidjson::Document> geoLocationDocument(response->getBodyAsJSON());

	if(geoLocationDocument == nullptr) {
		return {};
	}

	GeoLocation geoLocation;

	// parse ip address
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & ipAddressValue = (*geoLocationDocument)[JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME];

	if(!ipAddressValue.IsString()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected 'string'.", JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME, Utilities::typeToString(ipAddressValue.GetType()));
		return {};
	}

	geoLocation.ipAddress = ipAddressValue.GetString();

	if(geoLocation.ipAddress.empty()) {
		spdlog::error("Geo location data '{}' property cannot be empty.", JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME);
		return {};
	}

	// parse country name
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & countryNameValue = (*geoLocationDocument)[JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME];

	if(!countryNameValue.IsString()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected 'string'.", JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME, Utilities::typeToString(countryNameValue.GetType()));
		return {};
	}

	geoLocation.countryName = countryNameValue.GetString();

	if(geoLocation.countryName.empty()) {
		spdlog::error("Geo location data '{}' property cannot be empty.", JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME);
		return {};
	}

	// parse country code
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & countryCodeValue = (*geoLocationDocument)[JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME];

	if(!countryCodeValue.IsString()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected 'string'.", JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME, Utilities::typeToString(countryCodeValue.GetType()));
		return {};
	}

	geoLocation.countryCode = countryCodeValue.GetString();

	if(geoLocation.countryCode.empty()) {
		spdlog::error("Geo location data '{}' property cannot be empty.", JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME);
		return {};
	}

	// parse region name
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & regionNameValue = (*geoLocationDocument)[JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME];

	if(!regionNameValue.IsString()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected 'string'.", JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME, Utilities::typeToString(regionNameValue.GetType()));
		return {};
	}

	geoLocation.regionName = regionNameValue.GetString();

	if(geoLocation.regionName.empty()) {
		spdlog::error("Geo location data '{}' property cannot be empty.", JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME);
		return {};
	}

	// parse region code
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & regionCodeValue = (*geoLocationDocument)[JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME];

	if(!regionCodeValue.IsString()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected 'string'.", JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME, Utilities::typeToString(regionCodeValue.GetType()));
		return {};
	}

	geoLocation.regionCode = regionCodeValue.GetString();

	if(geoLocation.regionCode.empty()) {
		spdlog::error("Geo location data '{}' property cannot be empty.", JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME);
		return {};
	}

	// parse city
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & cityNameValue = (*geoLocationDocument)[JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME];

	if(!cityNameValue.IsString()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected 'string'.", JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME, Utilities::typeToString(cityNameValue.GetType()));
		return {};
	}

	geoLocation.cityName = cityNameValue.GetString();

	if(geoLocation.cityName.empty()) {
		spdlog::error("Geo location data '{}' property cannot be empty.", JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME);
		return {};
	}

	// parse time zone
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & timeZoneValue = (*geoLocationDocument)[JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME];

	if(!timeZoneValue.IsString()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected 'string'.", JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME, Utilities::typeToString(timeZoneValue.GetType()));
		return {};
	}

	geoLocation.timeZone = timeZoneValue.GetString();

	if(geoLocation.timeZone.empty()) {
		spdlog::error("Geo location data '{}' property cannot be empty.", JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME);
		return {};
	}

	// parse latitude
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & latitudeValue = (*geoLocationDocument)[JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME];

	if(!latitudeValue.IsDouble()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME, Utilities::typeToString(latitudeValue.GetType()));
		return {};
	}

	geoLocation.latitude = latitudeValue.GetDouble();

	// parse longitude
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME)) {
		spdlog::error("Geo location data is missing '{}' property'.", JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & longitudeValue = (*geoLocationDocument)[JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME];

	if(!longitudeValue.IsDouble()) {
		spdlog::error("Geo location data has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME, Utilities::typeToString(latitudeValue.GetType()));
		return {};
	}

	geoLocation.longitude = longitudeValue.GetDouble();

	return geoLocation;
}

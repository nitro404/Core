#include "FreeGeoIPGeoLocationService.h"

#include "Network/HTTPService.h"
#include "Utilities/RapidJSONUtilities.h"

#include <rapidjson/document.h>
#include <fmt/core.h>

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

	std::shared_ptr<HTTPService> httpService(getHTTPService());

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, FREE_GEO_IP_API_ADDRESS + "?" + API_KEY_QUERY_PARAMETER + "=" + getAPIKey()));
	request->setConnectionTimeout(3s);
	request->setNetworkTimeout(2s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		fmt::print("Failed to create Free GeoIP HTTP request!\n");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		fmt::print("Failed to retrieve geo location with error: {}\n", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		fmt::print("Failed to retrieve geo location ({}{})!\n", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::unique_ptr<rapidjson::Document> geoLocationDocument(response->getBodyAsJSON());

	fmt::print("{}\n", response->getBodyAsString());

	if(geoLocationDocument == nullptr) {
		return {};
	}

	GeoLocation geoLocation;

	// parse ip address
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & ipAddressValue = (*geoLocationDocument)[JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME];

	if(!ipAddressValue.IsString()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME, Utilities::typeToString(ipAddressValue.GetType()));
		return {};
	}

	geoLocation.ipAddress = ipAddressValue.GetString();

	if(geoLocation.ipAddress.empty()) {
		fmt::print("Geo location data '{}' property cannot be empty.\n", JSON_GEO_LOCATION_IP_ADDRESS_PROPERTY_NAME);
		return {};
	}

	// parse country name
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & countryNameValue = (*geoLocationDocument)[JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME];

	if(!countryNameValue.IsString()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME, Utilities::typeToString(countryNameValue.GetType()));
		return {};
	}

	geoLocation.countryName = countryNameValue.GetString();

	if(geoLocation.countryName.empty()) {
		fmt::print("Geo location data '{}' property cannot be empty.\n", JSON_GEO_LOCATION_COUNTRY_NAME_PROPERTY_NAME);
		return {};
	}

	// parse country code
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & countryCodeValue = (*geoLocationDocument)[JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME];

	if(!countryCodeValue.IsString()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME, Utilities::typeToString(countryCodeValue.GetType()));
		return {};
	}

	geoLocation.countryCode = countryCodeValue.GetString();

	if(geoLocation.countryCode.empty()) {
		fmt::print("Geo location data '{}' property cannot be empty.\n", JSON_GEO_LOCATION_COUNTRY_CODE_PROPERTY_NAME);
		return {};
	}

	// parse region name
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & regionNameValue = (*geoLocationDocument)[JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME];

	if(!regionNameValue.IsString()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME, Utilities::typeToString(regionNameValue.GetType()));
		return {};
	}

	geoLocation.regionName = regionNameValue.GetString();

	if(geoLocation.regionName.empty()) {
		fmt::print("Geo location data '{}' property cannot be empty.\n", JSON_GEO_LOCATION_REGION_NAME_PROPERTY_NAME);
		return {};
	}

	// parse region code
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & regionCodeValue = (*geoLocationDocument)[JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME];

	if(!regionCodeValue.IsString()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME, Utilities::typeToString(regionCodeValue.GetType()));
		return {};
	}

	geoLocation.regionCode = regionCodeValue.GetString();

	if(geoLocation.regionCode.empty()) {
		fmt::print("Geo location data '{}' property cannot be empty.\n", JSON_GEO_LOCATION_REGION_CODE_PROPERTY_NAME);
		return {};
	}

	// parse city
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & cityNameValue = (*geoLocationDocument)[JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME];

	if(!cityNameValue.IsString()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME, Utilities::typeToString(cityNameValue.GetType()));
		return {};
	}

	geoLocation.cityName = cityNameValue.GetString();

	if(geoLocation.cityName.empty()) {
		fmt::print("Geo location data '{}' property cannot be empty.\n", JSON_GEO_LOCATION_CITY_NAME_PROPERTY_NAME);
		return {};
	}

	// parse time zone
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & timeZoneValue = (*geoLocationDocument)[JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME];

	if(!timeZoneValue.IsString()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME, Utilities::typeToString(timeZoneValue.GetType()));
		return {};
	}

	geoLocation.timeZone = timeZoneValue.GetString();

	if(geoLocation.timeZone.empty()) {
		fmt::print("Geo location data '{}' property cannot be empty.\n", JSON_GEO_LOCATION_TIME_ZONE_PROPERTY_NAME);
		return {};
	}

	// parse latitude
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & latitudeValue = (*geoLocationDocument)[JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME];

	if(!latitudeValue.IsDouble()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_GEO_LOCATION_LATITUDE_PROPERTY_NAME, Utilities::typeToString(latitudeValue.GetType()));
		return {};
	}

	geoLocation.latitude = latitudeValue.GetDouble();

	// parse longitude
	if(!geoLocationDocument->HasMember(JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME)) {
		fmt::print("Geo location data is missing '{}' property'.\n", JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & longitudeValue = (*geoLocationDocument)[JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME];

	if(!longitudeValue.IsDouble()) {
		fmt::print("Geo location data has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_GEO_LOCATION_LONGITUDE_PROPERTY_NAME, Utilities::typeToString(latitudeValue.GetType()));
		return {};
	}

	geoLocation.longitude = longitudeValue.GetDouble();

	return geoLocation;
}

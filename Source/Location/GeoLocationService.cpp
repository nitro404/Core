#include "GeoLocationService.h"

#include "Network/HTTPService.h"

GeoLocationService::GeoLocationService()
	: m_initialized(false) { }

GeoLocationService::~GeoLocationService() { }

bool GeoLocationService::isInitialized() const {
	return m_initialized;
}

bool GeoLocationService::initialize(std::shared_ptr<HTTPService> httpService, const std::string & apiKey) {
	if(m_initialized || httpService == nullptr || !httpService->isInitialized() || apiKey.empty()) {
		return false;
	}

	m_httpService = httpService;
	m_apiKey = apiKey;

	m_initialized = true;

	return true;
}

const std::string & GeoLocationService::getAPIKey() const {
	return m_apiKey;
}

std::shared_ptr<HTTPService> GeoLocationService::getHTTPService() const {
	return m_httpService;
}

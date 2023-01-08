#include "GeoLocationService.h"

GeoLocationService::GeoLocationService()
	: m_initialized(false) { }

GeoLocationService::~GeoLocationService() { }

bool GeoLocationService::isInitialized() const {
	return m_initialized;
}

bool GeoLocationService::initialize(const std::string & apiKey) {
	if(m_initialized) {
		return true;
	}

	if(apiKey.empty()) {
		return false;
	}

	m_apiKey = apiKey;

	m_initialized = true;

	return true;
}

const std::string & GeoLocationService::getAPIKey() const {
	return m_apiKey;
}

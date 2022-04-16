#include "IPAddressService.h"

#include "Network/HTTPService.h"

IPAddressService::IPAddressService()
	: m_initialized(false) { }

IPAddressService::~IPAddressService() { }

bool IPAddressService::isInitialized() const {
	return m_initialized;
}

bool IPAddressService::initialize(std::shared_ptr<HTTPService> httpService) {
	if(m_initialized || httpService == nullptr || !httpService->isInitialized()) {
		return false;
	}

	m_httpService = httpService;

	m_initialized = true;

	return true;
}

std::shared_ptr<HTTPService> IPAddressService::getHTTPService() const {
	return m_httpService;
}

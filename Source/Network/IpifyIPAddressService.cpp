#include "IpifyIPAddressService.h"

#include "Network/HTTPService.h"

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

static const std::string IPIFY_IPV4_API_ADDRESS("https://api.ipify.org");
static const std::string IPIFY_IPV6_API_ADDRESS("https://api64.ipify.org");

IpifyIPAddressService::IpifyIPAddressService()
	: IPAddressService() { }

IpifyIPAddressService::~IpifyIPAddressService() { }

std::string IpifyIPAddressService::getIPAddress(IPAddressType type) {
	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::error("Failed to retrieve IP address, HTTP service is not initialized!");
		return {};
	}

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, type == IPAddressType::V4 ? IPIFY_IPV4_API_ADDRESS : IPIFY_IPV6_API_ADDRESS));
	request->setConnectionTimeout(2s);
	request->setNetworkTimeout(2s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create ipify HTTP request!");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve IP address with error: {}", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to retrieve IP address ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	return response->getBodyAsString();
}

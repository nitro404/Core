#include "IpifyIPAddressService.h"

#include "Network/HTTPService.h"

#include <fmt/core.h>

using namespace std::chrono_literals;

static const std::string IPIFY_IPV4_API_ADDRESS("https://api.ipify.org");
static const std::string IPIFY_IPV6_API_ADDRESS("https://api64.ipify.org");

IpifyIPAddressService::IpifyIPAddressService()
	: IPAddressService() { }

IpifyIPAddressService::~IpifyIPAddressService() { }

std::string IpifyIPAddressService::getIPAddress(IPAddressType type) {
	if(!isInitialized()) {
		return {};
	}

	std::shared_ptr<HTTPService> httpService(getHTTPService());

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, type == IPAddressType::V4 ? IPIFY_IPV4_API_ADDRESS : IPIFY_IPV6_API_ADDRESS));
	request->setConnectionTimeout(2s);
	request->setNetworkTimeout(2s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		fmt::print("Failed to create ipify HTTP request!\n");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		fmt::print("Failed to retrieve IP address with error: {}\n", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		fmt::print("Failed to retrieve IP address ({}{})!\n", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	return response->getBodyAsString();
}

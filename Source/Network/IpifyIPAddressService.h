#ifndef _IPIFY_IP_ADDRESS_SERVICE_H_
#define _IPIFY_IP_ADDRESS_SERVICE_H_

#include "IPAddressService.h"

#include <memory>

class HTTPService;

class IpifyIPAddressService final : public IPAddressService {
public:
	IpifyIPAddressService();
	~IpifyIPAddressService() override;

	virtual std::string getIPAddress(IPAddressType type = IPAddressType::V4) override;
};

#endif // _IPIFY_IP_ADDRESS_SERVICE_H_

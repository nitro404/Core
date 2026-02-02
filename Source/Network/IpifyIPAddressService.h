#ifndef _IPIFY_IP_ADDRESS_SERVICE_H_
#define _IPIFY_IP_ADDRESS_SERVICE_H_

#include "IPAddressService.h"

class HTTPService;

class IpifyIPAddressService final : public IPAddressService {
	friend class FactoryRegistry;

public:
	~IpifyIPAddressService() override;

	virtual std::string getIPAddress(IPAddressType type = IPAddressType::V4) override;

private:
	IpifyIPAddressService();
};

#endif // _IPIFY_IP_ADDRESS_SERVICE_H_

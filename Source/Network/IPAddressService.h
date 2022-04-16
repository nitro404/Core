#ifndef _IP_ADDRESS_SERVICE_H_
#define _IP_ADDRESS_SERVICE_H_

#include "Singleton/Singleton.h"

#include <memory>
#include <string>

class HTTPService;

class IPAddressService : public Singleton<IPAddressService> {
public:
	enum class IPAddressType {
		V4,
		V6
	};

	virtual ~IPAddressService();

	bool isInitialized() const;
	virtual bool initialize(std::shared_ptr<HTTPService> httpService);
	virtual std::string getIPAddress(IPAddressType type = IPAddressType::V4) = 0;

protected:
	IPAddressService();

	std::shared_ptr<HTTPService> getHTTPService() const;

private:
	bool m_initialized;
	std::shared_ptr<HTTPService> m_httpService;
};

#endif // _IP_ADDRESS_SERVICE_H_

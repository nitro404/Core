#ifndef _IP_ADDRESS_SERVICE_H_
#define _IP_ADDRESS_SERVICE_H_

#include "Singleton/Singleton.h"

#include <memory>
#include <string>

class IPAddressService : public Singleton<IPAddressService> {
public:
	enum class IPAddressType {
		V4,
		V6
	};

	virtual ~IPAddressService();

	virtual std::string getIPAddress(IPAddressType type = IPAddressType::V4) = 0;

protected:
	IPAddressService();
};

#endif // _IP_ADDRESS_SERVICE_H_

#include "FactoryRegistry.h"

#include "Application/ComponentRegistry.h"
#include "Network/IpifyIPAddressService.h"
#include "Location/FreeGeoIPGeoLocationService.h"

static std::unique_ptr<FactoryRegistry> s_factoryRegistryInstance;

FactoryRegistry::FactoryRegistry() { }

FactoryRegistry::~FactoryRegistry() = default;

FactoryRegistry & FactoryRegistry::getInstance() {
	if(s_factoryRegistryInstance == nullptr) {
		s_factoryRegistryInstance = std::unique_ptr<FactoryRegistry>(new FactoryRegistry());

		ComponentRegistry::getInstance().addComponent(&s_factoryRegistryInstance);
	}

	return *s_factoryRegistryInstance;
}

void FactoryRegistry::assignFactories() {
	assignStandardFactories();
	assignPlatformFactories();
}

void FactoryRegistry::assignStandardFactories() {
	setFactory<IPAddressService>([]() {
		return std::make_unique<IpifyIPAddressService>();
	});

	setFactory<GeoLocationService>([]() {
		return std::make_unique<FreeGeoIPGeoLocationService>();
	});
}

void FactoryRegistry::resetFactories() {
	m_factories.clear();
}

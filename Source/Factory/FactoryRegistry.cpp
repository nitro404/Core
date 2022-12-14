#include "FactoryRegistry.h"

#include "Application/ComponentRegistry.h"
#include "Archive/ArchiveFactoryRegistry.h"
#include "Network/IpifyIPAddressService.h"
#include "Location/FreeGeoIPGeoLocationService.h"
#include "Analytics/Segment/SegmentAnalyticsCURL.h"

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

	setFactory<SegmentAnalytics>([]() {
		return std::make_unique<SegmentAnalyticsCURL>();
	});

	setFactory<ArchiveFactoryRegistry>([]() {
		return std::make_unique<ArchiveFactoryRegistry>();
	});
}

void FactoryRegistry::resetFactories() {
	m_factories.clear();
}

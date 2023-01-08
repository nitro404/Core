#include "FactoryRegistry.h"

#include "Application/ComponentRegistry.h"
#include "Archive/ArchiveFactoryRegistry.h"
#include "GitHub/GitHubService.h"
#include "LibraryInformation.h"
#include "Logging/LogSystem.h"
#include "Network/HTTPService.h"
#include "Network/IpifyIPAddressService.h"
#include "Platform/TimeZoneDataManager.h"
#include "Location/FreeGeoIPGeoLocationService.h"
#include "Analytics/Segment/SegmentAnalyticsCURL.h"

static std::unique_ptr<FactoryRegistry> s_factoryRegistryInstance;

FactoryRegistry::FactoryRegistry()
	: m_defaultFactoriesAssigned(false) { }

FactoryRegistry::~FactoryRegistry() = default;

FactoryRegistry & FactoryRegistry::getInstance() {
	if(s_factoryRegistryInstance == nullptr) {
		s_factoryRegistryInstance = std::unique_ptr<FactoryRegistry>(new FactoryRegistry());

		ComponentRegistry::getInstance().addComponent(&s_factoryRegistryInstance);
	}

	return *s_factoryRegistryInstance;
}

void FactoryRegistry::resetFactories() {
	m_factories.clear();
}

bool FactoryRegistry::areDefaultFactoriesAssigned() const {
	return m_defaultFactoriesAssigned;
}

void FactoryRegistry::assignDefaultFactories() {
	if(m_defaultFactoriesAssigned) {
		return;
	}

	assignStandardFactories();
	assignPlatformFactories();

	LogSystem::getInstance();
	ArchiveFactoryRegistry::getInstance()->assignDefaultFactories();

	m_defaultFactoriesAssigned = true;
}

void FactoryRegistry::assignStandardFactories() {
	setFactory<IPAddressService>([]() {
		return std::make_unique<IpifyIPAddressService>();
	});

	setFactory<GeoLocationService>([]() {
		return std::make_unique<FreeGeoIPGeoLocationService>();
	});

	setFactory<GitHubService>([]() {
		return std::make_unique<GitHubService>();
	});

	setFactory<HTTPService>([]() {
		return std::make_unique<HTTPService>();
	});

	setFactory<LibraryInformation>([]() {
		return std::make_unique<LibraryInformation>();
	});

	setFactory<SegmentAnalytics>([]() {
		return std::make_unique<SegmentAnalyticsCURL>();
	});

	setFactory<TimeZoneDataManager>([]() {
		return std::make_unique<TimeZoneDataManager>();
	});

	setFactory<ArchiveFactoryRegistry>([]() {
		return std::make_unique<ArchiveFactoryRegistry>();
	});
}

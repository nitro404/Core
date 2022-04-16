#include "FactoryRegistry.h"

#include "Application/ComponentRegistry.h"

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

void FactoryRegistry::assignStandardFactories() { }

void FactoryRegistry::resetFactories() {
	m_factories.clear();
}

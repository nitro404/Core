#ifndef _FACTORY_REGISTRY_H_
#define _FACTORY_REGISTRY_H_

#include "Factory.h"

#include <functional>
#include <map>
#include <memory>

class FactoryRegistry final {
public:
	~FactoryRegistry();

	static FactoryRegistry & getInstance();

	template <class T>
	bool hasFactory() const;
	template <class T>
	std::function<std::unique_ptr<T>()> getFactory() const;
	template <class T>
	std::unique_ptr<T> callFactory() const;
	template <class T>
	void setFactory(std::function<std::unique_ptr<T>()> factory);
	void resetFactories();
	bool areDefaultFactoriesAssigned() const;
	void assignDefaultFactories();

private:
	typedef std::map<std::string, std::unique_ptr<Factory>> FactoryMap;

	FactoryRegistry();

	void assignStandardFactories();
	void assignPlatformFactories();

	FactoryMap m_factories;
	bool m_defaultFactoriesAssigned;

	FactoryRegistry(const FactoryRegistry &) = delete;
	FactoryRegistry(FactoryRegistry &&) noexcept = delete;
	const FactoryRegistry & operator = (const FactoryRegistry &) = delete;
	const FactoryRegistry & operator = (FactoryRegistry &&) noexcept = delete;
};

template <class T>
bool FactoryRegistry::hasFactory() const {
	return m_factories.find(typeid(T).name()) != m_factories.end();
}

template <class T>
std::function<std::unique_ptr<T>()> FactoryRegistry::getFactory() const {
	FactoryMap::const_iterator factory(m_factories.find(typeid(T).name()));

	if(factory == m_factories.end()) {
		return nullptr;
	}

	return factory->second->getFunction<T>();
}

template <class T>
std::unique_ptr<T> FactoryRegistry::callFactory() const {
	FactoryMap::const_iterator factory(m_factories.find(typeid(T).name()));

	if(factory == m_factories.end()) {
		return nullptr;
	}

	return factory->second->getFunction<T>()();
}

template <class T>
void FactoryRegistry::setFactory(std::function<std::unique_ptr<T>()> factory) {
	if(factory == nullptr) {
		m_factories.erase(typeid(T).name());
	}
	else {
		m_factories[typeid(T).name()] = std::make_unique<FactoryFunction<T>>(factory);
	}
}

#endif // _FACTORY_REGISTRY_H_

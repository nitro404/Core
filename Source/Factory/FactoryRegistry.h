#ifndef _FACTORY_REGISTRY_H_
#define _FACTORY_REGISTRY_H_

#include "Factory.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>

class FactoryRegistry final {
public:
	FactoryRegistry(FactoryRegistry && factoryRegistry) noexcept;
	const FactoryRegistry & operator = (FactoryRegistry && factoryRegistry) noexcept;
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
	mutable std::recursive_mutex m_mutex;

	FactoryRegistry(const FactoryRegistry &) = delete;
	const FactoryRegistry & operator = (const FactoryRegistry &) = delete;
};

template <class T>
bool FactoryRegistry::hasFactory() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_factories.find(typeid(T).name()) != m_factories.end();
}

template <class T>
std::function<std::unique_ptr<T>()> FactoryRegistry::getFactory() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	FactoryMap::const_iterator factory(m_factories.find(typeid(T).name()));

	if(factory == m_factories.end()) {
		return nullptr;
	}

	return factory->second->getFunction<T>();
}

template <class T>
std::unique_ptr<T> FactoryRegistry::callFactory() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	FactoryMap::const_iterator factory(m_factories.find(typeid(T).name()));

	if(factory == m_factories.end()) {
		return nullptr;
	}

	return factory->second->getFunction<T>()();
}

template <class T>
void FactoryRegistry::setFactory(std::function<std::unique_ptr<T>()> factory) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(factory == nullptr) {
		m_factories.erase(typeid(T).name());
	}
	else {
		m_factories[typeid(T).name()] = std::make_unique<FactoryFunction<T>>(factory);
	}
}

#endif // _FACTORY_REGISTRY_H_

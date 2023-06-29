#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include "Application/ComponentRegistry.h"
#include "SingletonManager.h"
#include "Factory/FactoryRegistry.h"

#include <typeinfo>

template <class T>
class Singleton {
public:
	virtual ~Singleton();

	static T * getInstance();
	static bool destroyInstance();

protected:
	Singleton();

private:
	uint64_t m_componentID;

	Singleton(const Singleton<T> &) = delete;
	const Singleton<T> & operator = (const Singleton<T> &) = delete;
};

template <class T>
Singleton<T>::Singleton() { }

template <class T>
Singleton<T>::~Singleton() { }

template <class T>
T * Singleton<T>::getInstance() {
	SingletonManager & singletonManager = SingletonManager::getInstance();

	T * instance = singletonManager.getSingleton<T>();

	if(instance != nullptr) {
		return instance;
	}

	std::function<std::unique_ptr<T>()> factory(FactoryRegistry::getInstance().getFactory<T>());

	if(factory == nullptr) {
		return nullptr;
	}

	std::unique_ptr<T> newSingleton(factory());
	T * rawNewSingleton = newSingleton.get();
	singletonManager.registerSingleton(rawNewSingleton);

	rawNewSingleton->m_componentID = ComponentRegistry::getInstance().addComponent(std::move(newSingleton));

	return rawNewSingleton;
}

template <class T>
bool Singleton<T>::destroyInstance() {
	SingletonManager & singletonManager = SingletonManager::getInstance();

	T * instance = singletonManager.getSingleton<T>();

	if(instance == nullptr) {
		return false;
	}

	bool unregistered = SingletonManager::getInstance().unregisterSingleton<T>();

	ComponentRegistry::getInstance().deleteComponent(instance->m_componentID);

	return unregistered;
}

#endif // _SINGLETON_H_

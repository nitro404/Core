#ifndef _SINGLETON_MANAGER_H_
#define _SINGLETON_MANAGER_H_

#include "Singleton.h"

#include <map>
#include <mutex>
#include <string>

class SingletonManager final {
public:
	~SingletonManager();

	static SingletonManager & getInstance();

	size_t numberOfSingletons() const;
	template <class T>
	bool hasSingleton() const;
	template <class T>
	const T * getSingleton() const;
	template <class T>
	T * getSingleton();
	template <class T>
	bool registerSingleton(T * instance);
	template <class T>
	bool unregisterSingleton();

private:
	typedef std::map<std::string, void *> SingletonMap;

	SingletonManager();

	SingletonMap m_singletons;
	mutable std::recursive_mutex m_mutex;

	SingletonManager(const SingletonManager &) = delete;
	SingletonManager(SingletonManager &&) noexcept = delete;
	const SingletonManager & operator = (const SingletonManager &) = delete;
	const SingletonManager & operator = (SingletonManager &&) noexcept = delete;
};

template <class T>
bool SingletonManager::hasSingleton() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SingletonMap::const_iterator i = m_singletons.find(typeid(T).name());

	return i != m_singletons.end();
}

template <class T>
const T * SingletonManager::getSingleton() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SingletonMap::const_iterator i = m_singletons.find(typeid(T).name());

	if(i == m_singletons.end()) {
		return nullptr;
	}

	return reinterpret_cast<const T *>(i->second);
}

template <class T>
T * SingletonManager::getSingleton() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SingletonMap::const_iterator i = m_singletons.find(typeid(T).name());

	if(i == m_singletons.end()) {
		return nullptr;
	}

	return reinterpret_cast<T *>(i->second);
}

template <class T>
bool SingletonManager::registerSingleton(T * singleton) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(hasSingleton<T>()) {
		return false;
	}

	m_singletons[typeid(T).name()] = singleton;

	return true;
}

template <class T>
bool SingletonManager::unregisterSingleton() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SingletonMap::iterator i = m_singletons.find(typeid(T).name());

	if(i == m_singletons.end()) {
		return false;
	}

	m_singletons.erase(i);

	return true;
}

#endif // _SINGLETON_MANAGER_H_

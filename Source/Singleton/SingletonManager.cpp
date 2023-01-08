#include "SingletonManager.h"

#include "Application/ComponentRegistry.h"

#include <memory>

std::unique_ptr<SingletonManager> s_singletonManagerInstance;

SingletonManager::SingletonManager() { }

SingletonManager::~SingletonManager() = default;

SingletonManager & SingletonManager::getInstance() {
	if(s_singletonManagerInstance == nullptr) {
		s_singletonManagerInstance = std::unique_ptr<SingletonManager>(new SingletonManager());

		ComponentRegistry::getInstance().addComponent(&s_singletonManagerInstance);
	}

	return *s_singletonManagerInstance;
}

size_t SingletonManager::numberOfSingletons() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_singletons.size();
}

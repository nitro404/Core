#include "ComponentRegistry.h"

static uint64_t s_componentIDCounter = 1;

ComponentRegistry::ComponentRegistry() { }

ComponentRegistry::~ComponentRegistry() = default;

ComponentRegistry & ComponentRegistry::getInstance() {
	static ComponentRegistry s_instance;

	return s_instance;
}

size_t ComponentRegistry::numberOfComponents() const {
	return m_components.size();
}

bool ComponentRegistry::hasComponent(uint64_t id) const {
	return m_components.find(id) != m_components.end();
}

bool ComponentRegistry::deleteComponent(uint64_t id) {
	bool componentFound = hasComponent(id);

	m_components.erase(id);

	return componentFound;
}

void ComponentRegistry::deleteAllComponents() {
	m_components.clear();
}

ComponentRegistry::Component::Component()
	: m_id(s_componentIDCounter++) { }

ComponentRegistry::Component::~Component() { }

uint64_t ComponentRegistry::Component::getID() const {
	return m_id;
}

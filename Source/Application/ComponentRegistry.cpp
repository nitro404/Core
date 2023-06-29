#include "ComponentRegistry.h"

#include "Network/HTTPUtilities.h"

#include <curl/curl.h>

static uint64_t s_componentIDCounter = 1;

ComponentRegistry::ComponentRegistry()
	: m_globalComponentsRegistered(false) {
	addGlobalComponent(std::make_unique<GlobalComponent>(std::bind(&ComponentRegistry::deleteAllComponents, this)));
}

ComponentRegistry::ComponentRegistry(ComponentRegistry && componentRegistry) noexcept
	: m_components(std::move(componentRegistry.m_components))
	, m_globalComponents(std::move(componentRegistry.m_globalComponents))
	, m_globalComponentsRegistered(componentRegistry.m_globalComponentsRegistered) { }

const ComponentRegistry & ComponentRegistry::operator = (ComponentRegistry && componentRegistry) noexcept {
	if(this != &componentRegistry) {
		m_components = std::move(componentRegistry.m_components);
		m_globalComponents = std::move(componentRegistry.m_globalComponents);
		m_globalComponentsRegistered = componentRegistry.m_globalComponentsRegistered;
	}

	return *this;
}

ComponentRegistry::~ComponentRegistry() {
	deleteAllGlobalComponents();
}

ComponentRegistry & ComponentRegistry::getInstance() {
	static ComponentRegistry s_instance;

	return s_instance;
}

size_t ComponentRegistry::numberOfComponents() const {
	std::lock_guard<std::recursive_mutex> lock(m_componentMutex);

	return m_components.size();
}

bool ComponentRegistry::hasComponent(uint64_t id) const {
	std::lock_guard<std::recursive_mutex> lock(m_componentMutex);

	return m_components.find(id) != m_components.end();
}

bool ComponentRegistry::deleteComponent(uint64_t id) {
	std::lock_guard<std::recursive_mutex> lock(m_componentMutex);

	return m_components.erase(id) != 0;
}

void ComponentRegistry::deleteAllComponents() {
	std::lock_guard<std::recursive_mutex> lock(m_componentMutex);

	m_components.clear();
}

size_t ComponentRegistry::numberOfGlobalComponents() const {
	std::lock_guard<std::recursive_mutex> lock(m_globalComponentMutex);

	return m_globalComponents.size();
}

bool ComponentRegistry::hasGlobalComponent(uint64_t id) const {
	std::lock_guard<std::recursive_mutex> lock(m_globalComponentMutex);

	return m_globalComponents.find(id) != m_globalComponents.end();
}

uint64_t ComponentRegistry::addGlobalComponent(std::unique_ptr<GlobalComponent> globalComponent) {
	std::lock_guard<std::recursive_mutex> lock(m_globalComponentMutex);

	if(globalComponent == nullptr) {
		return 0;
	}

	uint64_t globalComponentID = globalComponent->getID();
	m_globalComponents.emplace(globalComponentID, std::move(globalComponent));

	return globalComponentID;
}

bool ComponentRegistry::deleteGlobalComponent(uint64_t id) {
	std::lock_guard<std::recursive_mutex> lock(m_globalComponentMutex);

	return m_globalComponents.erase(id) != 0;
}

void ComponentRegistry::deleteAllGlobalComponents() {
	std::lock_guard<std::recursive_mutex> lock(m_globalComponentMutex);

	m_globalComponents.clear();
}

bool ComponentRegistry::areGlobalComponentsRegistered() const {
	return m_globalComponentsRegistered;
}

bool ComponentRegistry::registerGlobalComponents() {
	if(m_globalComponentsRegistered) {
		return true;
	}

	if(!registerStandardGlobalComponents()) {
		return false;
	}

	m_globalComponentsRegistered = true;

	return true;
}

bool ComponentRegistry::registerStandardGlobalComponents() {
	if(!HTTPUtilities::isSuccess(curl_global_init(CURL_GLOBAL_DEFAULT))) {
		return false;
	}

	addGlobalComponent(std::make_unique<GlobalComponent>([] {
		curl_global_cleanup();
	}));

	return true;
}

ComponentRegistry::Component::Component()
	: m_id(s_componentIDCounter++) { }

ComponentRegistry::Component::Component(Component && component) noexcept
	: m_id(component.m_id) { }

const ComponentRegistry::Component & ComponentRegistry::Component::operator = (Component && component) noexcept {
	if(this != &component) {
		m_id = component.m_id;
	}

	return *this;
}

ComponentRegistry::Component::~Component() { }

uint64_t ComponentRegistry::Component::getID() const {
	return m_id;
}

ComponentRegistry::GlobalComponent::GlobalComponent(std::function<void()> destroyFunction)
	: m_destroyFunction(destroyFunction) { }

ComponentRegistry::GlobalComponent::GlobalComponent(GlobalComponent && component) noexcept
	: Component(std::move(component))
	, m_destroyFunction(std::move(component.m_destroyFunction)) { }

const ComponentRegistry::GlobalComponent & ComponentRegistry::GlobalComponent::operator = (GlobalComponent && component) noexcept {
	if(this != &component) {
		Component::operator = (std::move(component));

		m_destroyFunction = std::move(component.m_destroyFunction);
	}

	return *this;
}

ComponentRegistry::GlobalComponent::~GlobalComponent() {
	if(m_destroyFunction) {
		m_destroyFunction();
	}
};

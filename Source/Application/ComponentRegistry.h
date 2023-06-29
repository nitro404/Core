#ifndef _COMPONENT_REGISTRY_H_
#define _COMPONENT_REGISTRY_H_

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

class ComponentRegistry final {
public:
	class GlobalComponent;

	ComponentRegistry(ComponentRegistry && componentRegistry) noexcept;
	const ComponentRegistry & operator = (ComponentRegistry && componentRegistry) noexcept;
	~ComponentRegistry();

	static ComponentRegistry & getInstance();

	size_t numberOfComponents() const;
	bool hasComponent(uint64_t id) const;
	template <class T>
	uint64_t addComponent(T ** component);
	template <class T>
	uint64_t addComponent(std::unique_ptr<T> component);
	template <class T>
	uint64_t addComponent(std::unique_ptr<T> * component);
	bool deleteComponent(uint64_t id);
	void deleteAllComponents();

	size_t numberOfGlobalComponents() const;
	bool hasGlobalComponent(uint64_t id) const;
	uint64_t addGlobalComponent(std::unique_ptr<GlobalComponent> globalComponent);
	bool deleteGlobalComponent(uint64_t id);
	void deleteAllGlobalComponents();

	bool areGlobalComponentsRegistered() const;
	bool registerGlobalComponents();

private:
	class Component {
	public:
		Component();
		Component(Component && component) noexcept;
		const Component & operator = (Component && component) noexcept;
		virtual ~Component();

		uint64_t getID() const;

	private:
		uint64_t m_id;

		Component(const Component &) = delete;
		const Component & operator = (const Component &) = delete;
	};

	template <class T>
	class RawComponent final : public Component {
	public:
		RawComponent(T ** component);
		RawComponent(RawComponent<T> && component) noexcept;
		const RawComponent<T> & operator = (RawComponent<T> && component) noexcept;
		virtual ~RawComponent();

	private:
		T ** m_component;

		RawComponent(const RawComponent<T> &) = delete;
		const RawComponent<T> & operator = (const RawComponent<T> &) = delete;
	};

	template <class T>
	class OwnedUniqueComponent final : public Component {
	public:
		OwnedUniqueComponent(std::unique_ptr<T> component);
		OwnedUniqueComponent(OwnedUniqueComponent<T> && component) noexcept;
		const OwnedUniqueComponent<T> & operator = (OwnedUniqueComponent<T> && component) noexcept;
		virtual ~OwnedUniqueComponent();

	private:
		std::unique_ptr<T> m_component;

		OwnedUniqueComponent(const OwnedUniqueComponent<T> &) = delete;
		const OwnedUniqueComponent<T> & operator = (const OwnedUniqueComponent<T> &) = delete;
	};

	template <class T>
	class ExternalUniqueComponent final : public Component {
	public:
		ExternalUniqueComponent(std::unique_ptr<T> * component);
		ExternalUniqueComponent(ExternalUniqueComponent<T> && component) noexcept;
		const ExternalUniqueComponent<T> & operator = (ExternalUniqueComponent<T> && component) noexcept;
		virtual ~ExternalUniqueComponent();

	private:
		std::unique_ptr<T> * m_component;

		ExternalUniqueComponent(const ExternalUniqueComponent<T> &) = delete;
		const ExternalUniqueComponent<T> & operator = (const ExternalUniqueComponent<T> &) = delete;
	};

	class GlobalComponent final : public Component {
	public:
		GlobalComponent(std::function<void()> destroyFunction);
		GlobalComponent(GlobalComponent && component) noexcept;
		const GlobalComponent & operator = (GlobalComponent && component) noexcept;
		virtual ~GlobalComponent();

	private:
		std::function<void()> m_destroyFunction;

		GlobalComponent(const GlobalComponent &) = delete;
		const GlobalComponent & operator = (const GlobalComponent &) = delete;
	};

	ComponentRegistry();

	bool registerStandardGlobalComponents();

	std::map<uint64_t, std::unique_ptr<Component>> m_components;
	std::map<uint64_t, std::unique_ptr<GlobalComponent>> m_globalComponents;
	bool m_globalComponentsRegistered;
	mutable std::recursive_mutex m_componentMutex;
	mutable std::recursive_mutex m_globalComponentMutex;

	ComponentRegistry(const ComponentRegistry &) = delete;
	const ComponentRegistry & operator = (const ComponentRegistry &) = delete;
};

template <class T>
uint64_t ComponentRegistry::addComponent(T ** component) {
	std::lock_guard<std::recursive_mutex> lock(m_componentMutex);

	if(component == nullptr) {
		return 0;
	}

	std::unique_ptr<RawComponent<T>> rawComponent(std::make_unique<RawComponent<T>>(component));
	uint64_t componentID = rawComponent->getID();

	m_components.emplace(componentID, std::move(rawComponent));

	return componentID;
}

template <class T>
uint64_t ComponentRegistry::addComponent(std::unique_ptr<T> component) {
	std::lock_guard<std::recursive_mutex> lock(m_componentMutex);

	if(component == nullptr) {
		return 0;
	}

	std::unique_ptr<OwnedUniqueComponent<T>> ownedUniqueComponent(std::make_unique<OwnedUniqueComponent<T>>(std::move(component)));
	uint64_t componentID = ownedUniqueComponent->getID();

	m_components.emplace(componentID, std::move(ownedUniqueComponent));

	return componentID;
}

template <class T>
uint64_t ComponentRegistry::addComponent(std::unique_ptr<T> * component) {
	std::lock_guard<std::recursive_mutex> lock(m_componentMutex);

	if(component == nullptr) {
		return 0;
	}

	std::unique_ptr<ExternalUniqueComponent<T>> externalUniqueComponent(std::make_unique<ExternalUniqueComponent<T>>(component));
	uint64_t componentID = externalUniqueComponent->getID();

	m_components.emplace(componentID, std::move(externalUniqueComponent));

	return componentID;
}

template <class T>
ComponentRegistry::RawComponent<T>::RawComponent(T ** component)
	: Component()
	, m_component(component) { }

template <class T>
ComponentRegistry::RawComponent<T>::RawComponent(RawComponent && component) noexcept
	: Component(std::move(component))
	, m_component(component.m_component) { }

template <class T>
const ComponentRegistry::RawComponent<T> & ComponentRegistry::RawComponent<T>::operator = (RawComponent<T> && component) noexcept {
	if(this != &component) {
		Component::operator = std::move(component);

		m_component = component.m_component;
	}

	return *this;
}

template <class T>
ComponentRegistry::RawComponent<T>::~RawComponent() {
	if(m_component != nullptr && *m_component != nullptr) {
		delete *m_component;
		*m_component = nullptr;
	}
}

template <class T>
ComponentRegistry::OwnedUniqueComponent<T>::OwnedUniqueComponent(std::unique_ptr<T> component)
	: Component()
	, m_component(std::move(component)) { }

template <class T>
ComponentRegistry::OwnedUniqueComponent<T>::OwnedUniqueComponent(OwnedUniqueComponent && component) noexcept
	: Component(std::move(component))
	, m_component(std::move(component.m_component)) { }

template <class T>
const ComponentRegistry::OwnedUniqueComponent<T> & ComponentRegistry::OwnedUniqueComponent<T>::operator = (OwnedUniqueComponent<T> && component) noexcept {
	if(this != &component) {
		Component::operator = std::move(component);

		m_component = std::move(component.m_component);
	}

	return *this;
}

template <class T>
ComponentRegistry::OwnedUniqueComponent<T>::~OwnedUniqueComponent() {
	m_component.reset();
}

template <class T>
ComponentRegistry::ExternalUniqueComponent<T>::ExternalUniqueComponent(std::unique_ptr<T> * component)
	: Component()
	, m_component(component) { }

template <class T>
ComponentRegistry::ExternalUniqueComponent<T>::ExternalUniqueComponent(ExternalUniqueComponent && component) noexcept
	: Component(std::move(component))
	, m_component(component.m_component) { }

template <class T>
const ComponentRegistry::ExternalUniqueComponent<T> & ComponentRegistry::ExternalUniqueComponent<T>::operator = (ExternalUniqueComponent<T> && component) noexcept {
	if(this != &component) {
		Component::operator = std::move(component);

		m_component = component.m_component;
	}

	return *this;
}

template <class T>
ComponentRegistry::ExternalUniqueComponent<T>::~ExternalUniqueComponent() {
	if(m_component != nullptr) {
		m_component->reset();
	}
}

#endif // _COMPONENT_REGISTRY_H_

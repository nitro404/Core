#include "Application.h"

#include "ComponentRegistry.h"
#include "Factory/FactoryRegistry.h"

Application::Application() {
	FactoryRegistry::getInstance().assignFactories();
}

Application::~Application() {
	ComponentRegistry::getInstance().deleteAllComponents();
}

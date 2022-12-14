#include "Application.h"

#include "ComponentRegistry.h"
#include "Archive/ArchiveFactoryRegistry.h"
#include "Factory/FactoryRegistry.h"
#include "Logging/LogSystem.h"

Application::Application() {
	FactoryRegistry::getInstance().assignFactories();
	ArchiveFactoryRegistry::getInstance()->assignFactories();

	LogSystem::getInstance();
}

Application::~Application() {
	ComponentRegistry::getInstance().deleteAllComponents();
}

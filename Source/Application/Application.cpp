#include "Application.h"

#include "ComponentRegistry.h"
#include "Archive/ArchiveFactoryRegistry.h"
#include "Factory/FactoryRegistry.h"
#include "Logging/LogSystem.h"
#include "Logging/Provider/LogProviderCDIO.h"

Application::Application() {
	FactoryRegistry::getInstance().assignDefaultFactories();

	LogProviderCDIO::getInstance()->initialize();
}

Application::~Application() { }

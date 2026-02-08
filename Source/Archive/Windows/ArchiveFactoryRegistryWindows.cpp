#include "Archive/ArchiveFactoryRegistry.h"

#include "Archive/NSIS/NullsoftScriptableInstallSystemArchive.h"

void ArchiveFactoryRegistry::assignPlatformFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setFactory(NullsoftScriptableInstallSystemArchive::DEFAULT_FILE_EXTENSION, static_cast<bool(*)(const ByteBuffer &)>(&NullsoftScriptableInstallSystemArchive::isNSISArchive), [](std::unique_ptr<ByteBuffer> buffer) {
		return NullsoftScriptableInstallSystemArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return NullsoftScriptableInstallSystemArchive::readFrom(filePath);
	});
}

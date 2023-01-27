#include "ArchiveFactoryRegistry.h"

#include "Archive/7Zip/SevenZipArchive.h"
#include "Archive/Rar/RarArchive.h"
#include "Archive/Tar/TarArchive.h"
#include "Archive/Tar/TarBZip2Archive.h"
#include "Archive/Tar/TarGZipArchive.h"
#include "Archive/Tar/TarLZMAArchive.h"
#include "Archive/Tar/TarXZArchive.h"
#include "Archive/Tar/TarZStandardArchive.h"
#include "Archive/Zip/ZipArchive.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

ArchiveFactoryRegistry::ArchiveFactoryRegistry()
	: m_defaultFactoriesAssigned(false) { }

ArchiveFactoryRegistry::~ArchiveFactoryRegistry() { }

bool ArchiveFactoryRegistry::hasFactory(const std::string & fileExtension) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_archiveFactories.find(ArchiveFactoryRegistry::formatFileExtension(fileExtension)) != m_archiveFactories.end();
}

bool ArchiveFactoryRegistry::setFactory(const std::string & fileExtension, std::function<std::unique_ptr<Archive>(std::unique_ptr<ByteBuffer> buffer)> createArchiveFunction, std::function<std::unique_ptr<Archive>(const std::string & filePath)> readArchiveFunction) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(fileExtension.empty() || createArchiveFunction == nullptr || readArchiveFunction == nullptr) {
		return false;
	}

	std::string formattedFileExtension(formatFileExtension(fileExtension));

	if(formattedFileExtension.empty()) {
		return false;
	}

	m_archiveFactories.emplace(formattedFileExtension, ArchiveFactoryData{
		createArchiveFunction,
		readArchiveFunction
	});

	return true;
}

size_t ArchiveFactoryRegistry::setFactory(const std::vector<std::string> & fileExtensions, std::function<std::unique_ptr<Archive>(std::unique_ptr<ByteBuffer> buffer)> createArchiveFunction, std::function<std::unique_ptr<Archive>(const std::string & filePath)> readArchiveFunction) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	size_t numberOfFactoriesSet = 0;

	for(const std::string & fileExtension : fileExtensions) {
		if(setFactory(fileExtension, createArchiveFunction, readArchiveFunction)) {
			numberOfFactoriesSet++;
		}
	}

	return numberOfFactoriesSet;
}

bool ArchiveFactoryRegistry::areDefaultFactoriesAssigned() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_defaultFactoriesAssigned;
}

void ArchiveFactoryRegistry::assignDefaultFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_defaultFactoriesAssigned) {
		return;
	}

	assignStandardFactories();
	assignPlatformFactories();

	m_defaultFactoriesAssigned = true;
}

bool ArchiveFactoryRegistry::removeFactory(const std::string & fileExtension) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(fileExtension.empty()) {
		return false;
	}

	std::string formattedFileExtension(formatFileExtension(fileExtension));

	if(formattedFileExtension.empty()) {
		return false;
	}

	ArchiveFactoryMap::const_iterator factoryDataIterator(m_archiveFactories.find(formattedFileExtension));

	if(factoryDataIterator == m_archiveFactories.cend()) {
		return false;
	}

	m_archiveFactories.erase(factoryDataIterator);

	return true;
}

void ArchiveFactoryRegistry::resetFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_archiveFactories.clear();
}

void ArchiveFactoryRegistry::assignStandardFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setFactory(RarArchive::DEFAULT_FILE_EXTENSION, [](std::unique_ptr<ByteBuffer> buffer) {
		return RarArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return RarArchive::readFrom(filePath);
	});

	setFactory(SevenZipArchive::DEFAULT_FILE_EXTENSION, [](std::unique_ptr<ByteBuffer> buffer) {
		return SevenZipArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return SevenZipArchive::readFrom(filePath);
	});

	setFactory(TarArchive::DEFAULT_FILE_EXTENSION, [](std::unique_ptr<ByteBuffer> buffer) {
		return TarArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return TarArchive::readFrom(filePath);
	});

	setFactory(TarLZMAArchive::FILE_EXTENSIONS, [](std::unique_ptr<ByteBuffer> buffer) {
		return TarLZMAArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return TarLZMAArchive::readFrom(filePath);
	});

	setFactory(TarBZip2Archive::FILE_EXTENSIONS, [](std::unique_ptr<ByteBuffer> buffer) {
		return TarBZip2Archive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return TarBZip2Archive::readFrom(filePath);
	});

	setFactory(TarGZipArchive::FILE_EXTENSIONS, [](std::unique_ptr<ByteBuffer> buffer) {
		return TarGZipArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return TarGZipArchive::readFrom(filePath);
	});

	setFactory(TarXZArchive::FILE_EXTENSIONS, [](std::unique_ptr<ByteBuffer> buffer) {
		return TarXZArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return TarXZArchive::readFrom(filePath);
	});

	setFactory(TarZStandardArchive::FILE_EXTENSIONS, [](std::unique_ptr<ByteBuffer> buffer) {
		return TarZStandardArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return TarZStandardArchive::readFrom(filePath);
	});

	setFactory(ZipArchive::DEFAULT_FILE_EXTENSION, [](std::unique_ptr<ByteBuffer> buffer) {
		return ZipArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return ZipArchive::readFrom(filePath);
	});
}

ArchiveFactoryRegistry::ArchiveFactoryMap::const_iterator ArchiveFactoryRegistry::getArchiveFactoryForFilePath(const std::string & filePath) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(filePath.empty()) {
		return m_archiveFactories.cend();
	}

	std::string formattedFileExtension(formatFileExtension(std::string(Utilities::getFileExtension(filePath))));

	if(formattedFileExtension.empty()) {
		return m_archiveFactories.cend();
	}

	ArchiveFactoryMap::const_iterator archiveFactoryIterator(m_archiveFactories.find(formattedFileExtension));

	if(archiveFactoryIterator != m_archiveFactories.cend()) {
		return archiveFactoryIterator;
	}

	return std::find_if(m_archiveFactories.cbegin(), m_archiveFactories.cend(), [&filePath](const auto & archiveFactory) {
		return Utilities::endsWith(filePath, archiveFactory.first);
	});
}

std::unique_ptr<Archive> ArchiveFactoryRegistry::createArchiveFrom(std::unique_ptr<ByteBuffer> buffer, const std::string & filePath) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	ArchiveFactoryMap::const_iterator archiveFactoryIterator(getArchiveFactoryForFilePath(filePath));

	if(archiveFactoryIterator == m_archiveFactories.cend()) {
		return nullptr;
	}

	return archiveFactoryIterator->second.createArchiveFunction(std::move(buffer));
}

std::unique_ptr<Archive> ArchiveFactoryRegistry::readArchiveFrom(const std::string & filePath) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	ArchiveFactoryMap::const_iterator archiveFactoryIterator(getArchiveFactoryForFilePath(filePath));

	if(archiveFactoryIterator == m_archiveFactories.cend()) {
		return nullptr;
	}

	return archiveFactoryIterator->second.readArchiveFunction(filePath);
}

std::string ArchiveFactoryRegistry::formatFileExtension(const std::string & fileExtension) {
	return Utilities::toLowerCase(Utilities::trimString(fileExtension));
}
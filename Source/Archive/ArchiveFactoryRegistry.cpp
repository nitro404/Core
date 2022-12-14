#include "ArchiveFactoryRegistry.h"

#include "Archive/7Zip/SevenZipArchive.h"
#include "Archive/Rar/RarArchive.h"
#include "Archive/Zip/ZipArchive.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

ArchiveFactoryRegistry::ArchiveFactoryRegistry() { }

ArchiveFactoryRegistry::~ArchiveFactoryRegistry() { }

bool ArchiveFactoryRegistry::hasFactory(const std::string & fileExtension) const {
	return m_archiveFactories.find(ArchiveFactoryRegistry::formatFileExtension(fileExtension)) != m_archiveFactories.end();
}

bool ArchiveFactoryRegistry::setFactory(const std::string & fileExtension, std::function<std::unique_ptr<Archive>(std::unique_ptr<ByteBuffer> buffer)> createArchiveFunction, std::function<std::unique_ptr<Archive>(const std::string & filePath)> readArchiveFunction) {
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

void ArchiveFactoryRegistry::assignFactories() {
	assignStandardFactories();
}

bool ArchiveFactoryRegistry::removeFactory(const std::string & fileExtension) {
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
	m_archiveFactories.clear();
}

void ArchiveFactoryRegistry::assignStandardFactories() {
	setFactory(ZipArchive::DEFAULT_FILE_EXTENSION, [](std::unique_ptr<ByteBuffer> buffer) {
		return ZipArchive::createFrom(std::move(buffer));
	}, [](const std::string & filePath) {
		return ZipArchive::readFrom(filePath);
	});

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
}

std::unique_ptr<Archive> ArchiveFactoryRegistry::createArchiveFrom(std::unique_ptr<ByteBuffer> buffer, const std::string & fileExtension) {
	if(fileExtension.empty()) {
		return nullptr;
	}

	std::string formattedFileExtension(formatFileExtension(fileExtension));

	if(formattedFileExtension.empty()) {
		return nullptr;
	}

	ArchiveFactoryMap::const_iterator factoryDataIterator(m_archiveFactories.find(formattedFileExtension));

	if(factoryDataIterator == m_archiveFactories.cend()) {
		return nullptr;
	}

	return factoryDataIterator->second.createArchiveFunction(std::move(buffer));
}

std::unique_ptr<Archive> ArchiveFactoryRegistry::readArchiveFrom(const std::string & filePath) {
	if(filePath.empty()) {
		return nullptr;
	}

	std::string formattedFileExtension(formatFileExtension(std::string(Utilities::getFileExtension(filePath))));

	if(formattedFileExtension.empty()) {
		return nullptr;
	}

	ArchiveFactoryMap::const_iterator factoryDataIterator(m_archiveFactories.find(formattedFileExtension));

	if(factoryDataIterator == m_archiveFactories.cend()) {
		return nullptr;
	}

	return factoryDataIterator->second.readArchiveFunction(filePath);
}

std::string ArchiveFactoryRegistry::formatFileExtension(const std::string & fileExtension) {
	return Utilities::toLowerCase(Utilities::trimString(fileExtension));
}
#include "NullsoftScriptableInstallSystemArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <spdlog/spdlog.h>

NullsoftScriptableInstallSystemArchive::Entry::~Entry() = default;

NullsoftScriptableInstallSystemArchive::Entry::Entry(uint64_t index, NullsoftScriptableInstallSystemArchive * parentArchive)
	: m_index(index)
	, m_parentArchive(parentArchive) { }

bool NullsoftScriptableInstallSystemArchive::Entry::isFile() const {
// TODO:
return false;
}

bool NullsoftScriptableInstallSystemArchive::Entry::isDirectory() const {
// TODO:
return false;
}

std::string NullsoftScriptableInstallSystemArchive::Entry::getPath() const {
	return m_path;
}

uint64_t NullsoftScriptableInstallSystemArchive::Entry::getIndex() const {
	return m_index;
}

bool NullsoftScriptableInstallSystemArchive::Entry::hasComment() const {
	return false;
}

std::string NullsoftScriptableInstallSystemArchive::Entry::getComment() const {
	return {};
}

uint64_t NullsoftScriptableInstallSystemArchive::Entry::getCompressedSize() const {
// TODO:
return 0;
}

uint64_t NullsoftScriptableInstallSystemArchive::Entry::getUncompressedSize() const {
// TODO:
return 0;
}

std::chrono::time_point<std::chrono::system_clock> NullsoftScriptableInstallSystemArchive::Entry::getDate() const {
// TODO:
return {};
}

std::unique_ptr<ByteBuffer> NullsoftScriptableInstallSystemArchive::Entry::getData() const {
// TODO:
return nullptr;
}

uint32_t NullsoftScriptableInstallSystemArchive::Entry::getCRC32() const {
// TODO:
return 0;
}

bool NullsoftScriptableInstallSystemArchive::Entry::writeTo(const std::string & directoryPath, bool overwrite) const {
	std::unique_ptr<ByteBuffer> data(getData());
	std::string path(getPath());

	if(data == nullptr) {
		spdlog::error("Failed to obtain NullSoft Install System installer entry file data when writing entry '{}' to directory: '{}'.", path, directoryPath);
		return false;
	}

	return data->writeTo(Utilities::joinPaths(directoryPath, path), overwrite);
}

Archive * NullsoftScriptableInstallSystemArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

bool NullsoftScriptableInstallSystemArchive::Entry::setParentArchive(Archive * archive) {
	if(archive == nullptr) {
		m_parentArchive = nullptr;
		return true;
	}

	NullsoftScriptableInstallSystemArchive * nsisArchive = dynamic_cast<NullsoftScriptableInstallSystemArchive *>(archive);

	if(nsisArchive == nullptr) {
		return false;
	}

	m_parentArchive = nsisArchive;

	return true;
}

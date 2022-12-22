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

bool NullsoftScriptableInstallSystemArchive::Entry::isInSubdirectory() const {
	return isInSubdirectory(getPath());
}

bool NullsoftScriptableInstallSystemArchive::Entry::isInSubdirectory(std::string_view path) {
	if(path.empty()) {
		return false;
	}

	return path.find_first_of("/") < path.length() - 1;
}

std::string NullsoftScriptableInstallSystemArchive::Entry::getName() const {
	std::string filePath(getPath());

	if(isFile()) {
		return std::string(Utilities::getFileName(filePath));
	}

	return std::string(Utilities::getFileName(Utilities::trimTrailingPathSeparator(filePath)));
}

std::string NullsoftScriptableInstallSystemArchive::Entry::getPath() const {
	return m_path;
}

std::vector<std::weak_ptr<NullsoftScriptableInstallSystemArchive::Entry>> NullsoftScriptableInstallSystemArchive::Entry::getChildren(bool includeSubdirectories, bool caseSensitive) const {
	if(!isParentArchiveValid() || !isDirectory()) {
		return {};
	}

	std::vector<std::weak_ptr<Entry>> children;
	const std::vector<std::shared_ptr<Entry>> & entries = m_parentArchive->getEntries();

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = entries.begin(); i != entries.end(); ++i) {
		if(*i == nullptr || (*i).get() == this) {
			continue;
		}

		const std::string & currentPath = (*i)->getPath();
		size_t firstPathSeparatorIndex = currentPath.find_first_of("/");

		std::string entryBasePath;

		if(firstPathSeparatorIndex != std::string::npos && firstPathSeparatorIndex != currentPath.length() - 1) {
			entryBasePath = Utilities::addTrailingPathSeparator(Utilities::getFilePath(Utilities::trimTrailingPathSeparator(currentPath)));
		}

		if(entryBasePath.empty()) {
			continue;
		}

		std::string path(getPath());

		if(includeSubdirectories) {
			if(entryBasePath.length() < path.length()) {
				continue;
			}

			if(Utilities::areStringsEqual(std::string_view(entryBasePath.data(), path.length()), path, caseSensitive)) {
				children.push_back(*i);
			}
		}
		else {
			if(Utilities::areStringsEqual(entryBasePath, path, caseSensitive)) {
				children.push_back(*i);
			}
		}
	}

	return children;
}

uint64_t NullsoftScriptableInstallSystemArchive::Entry::getIndex() const {
	return m_index;
}

std::chrono::time_point<std::chrono::system_clock> NullsoftScriptableInstallSystemArchive::Entry::getDate() const {
// TODO:
return {};
}

uint64_t NullsoftScriptableInstallSystemArchive::Entry::getInflatedSize() const {
// TODO:
return 0;
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

NullsoftScriptableInstallSystemArchive * NullsoftScriptableInstallSystemArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

void NullsoftScriptableInstallSystemArchive::Entry::clearParentArchive() {
	m_parentArchive = nullptr;
}

bool NullsoftScriptableInstallSystemArchive::Entry::isParentArchiveValid() const {
	return m_parentArchive != nullptr;
}

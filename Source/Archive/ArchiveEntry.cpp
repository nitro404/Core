#include "ArchiveEntry.h"

#include "Archive.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

ArchiveEntry::ArchiveEntry() { }

ArchiveEntry::ArchiveEntry(ArchiveEntry && e) noexcept { }

ArchiveEntry::ArchiveEntry(const ArchiveEntry & e) { }

ArchiveEntry & ArchiveEntry::operator = (ArchiveEntry && e) noexcept {
	return *this;
}

ArchiveEntry & ArchiveEntry::operator = (const ArchiveEntry & e) {
	return *this;
}

ArchiveEntry::~ArchiveEntry() { }

bool ArchiveEntry::isFile() const {
	return !isDirectory();
}

bool ArchiveEntry::isDirectory() const {
	return isDirectory(getPath());
}

bool ArchiveEntry::isDirectory(std::string_view path) {
	if(path.empty()) {
		return false;
	}

	return path[path.length() - 1] == '/';
}

bool ArchiveEntry::isInSubdirectory() const {
	return isInSubdirectory(getPath());
}

bool ArchiveEntry::isInSubdirectory(std::string_view path) {
	if(path.empty()) {
		return false;
	}

	return path.find_first_of("/") < path.length() - 1;
}

std::string ArchiveEntry::getName() const {
	std::string filePath(getPath());

	if(isFile()) {
		return std::string(Utilities::getFileName(filePath));
	}

	return std::string(Utilities::getFileName(Utilities::trimTrailingPathSeparator(filePath)));
}

std::vector<std::shared_ptr<ArchiveEntry>> ArchiveEntry::getChildren(bool includeSubdirectories, bool caseSensitive) const {
	if(!isParentArchiveValid() || !isDirectory()) {
		return {};
	}

	std::vector<std::shared_ptr<ArchiveEntry>> children;
	const std::vector<std::shared_ptr<ArchiveEntry>> & entries = getParentArchive()->getEntries();

	for(std::vector<std::shared_ptr<ArchiveEntry>>::const_iterator i = entries.begin(); i != entries.end(); ++i) {
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

bool ArchiveEntry::hasComment() const {
	return !getComment().empty();
}

bool ArchiveEntry::isParentArchiveValid() const {
	return getParentArchive() != nullptr;
}

void ArchiveEntry::clearParentArchive() {
	setParentArchive(nullptr);
}

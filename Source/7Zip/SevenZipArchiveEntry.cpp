#include "SevenZipArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <7Zip/C/7z.h>

#include <spdlog/spdlog.h>

SevenZipArchive::Entry::Entry(uint64_t index, SevenZipArchive * parentArchive)
	: m_index(index)
	, m_parentArchive(parentArchive) { }

SevenZipArchive::Entry::~Entry() = default;

bool SevenZipArchive::Entry::isFile() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	return !SzArEx_IsDir(m_parentArchive->getRawArchiveHandle(), m_index);
}

bool SevenZipArchive::Entry::isInSubdirectory() const {
	return isInSubdirectory(getPath());
}

bool SevenZipArchive::Entry::isInSubdirectory(std::string_view path) {
	if(path.empty()) {
		return false;
	}

	return path.find_first_of("/") < path.length() - 1;
}

bool SevenZipArchive::Entry::isDirectory() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	return SzArEx_IsDir(m_parentArchive->getRawArchiveHandle(), m_index);
}

std::string SevenZipArchive::Entry::getName() const {
	std::string filePath(getPath());

	if(isFile()) {
		return std::string(Utilities::getFileName(filePath));
	}

	return std::string(Utilities::getFileName(Utilities::trimTrailingPathSeparator(filePath)));
}

std::string SevenZipArchive::Entry::getPath() const {
	if(!isParentArchiveValid()) {
		return Utilities::emptyString;
	}

	const CSzArEx * archiveHandle = m_parentArchive->getRawArchiveHandle();

	if(m_index >= archiveHandle->NumFiles) {
		return Utilities::emptyString;
	}

	size_t offset = archiveHandle->FileNameOffsets[m_index];

	return Utilities::wideStringToString(std::wstring(reinterpret_cast<const wchar_t *>(archiveHandle->FileNames + (offset * 2)), archiveHandle->FileNameOffsets[m_index + 1] - offset - 1)).append(isDirectory() ? "/" : "");
}

std::vector<std::weak_ptr<SevenZipArchive::Entry>> SevenZipArchive::Entry::getChildren(bool includeSubdirectories, bool caseSensitive) const {
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

uint64_t SevenZipArchive::Entry::getIndex() const {
	return m_index;
}

std::chrono::time_point<std::chrono::system_clock> SevenZipArchive::Entry::getDate() const {
	const CSzArEx * archiveHandle = m_parentArchive->getRawArchiveHandle();

	if(!SzBitWithVals_Check(&archiveHandle->MTime, m_index)) {
		return std::chrono::system_clock::from_time_t(time_t{0});
	}

	return SevenZipArchive::getTimePointFromNTFSFileTime(archiveHandle->MTime.Vals[m_index]);
}

uint64_t SevenZipArchive::Entry::getInflatedSize() const {
	if(!isParentArchiveValid()) {
		return 0;
	}

	return SzArEx_GetFileSize(m_parentArchive->getRawArchiveHandle(), m_index);
};

std::unique_ptr<ByteBuffer> SevenZipArchive::Entry::getData() const {
	if(!isParentArchiveValid()) {
		return nullptr;
	}

	ExtractionData & extractionData = m_parentArchive->getCachedExtractionData();

	size_t offset = 0;
	size_t outputSizeProcessed = 0;
	ISzAlloc temporaryAllocator = SevenZipArchive::DEFAULT_ALLOCATOR;

	if(SzArEx_Extract(m_parentArchive->getRawArchiveHandle(), &m_parentArchive->getRawLookStreamHandle()->vt, m_index, &extractionData.blockIndex, &extractionData.outputBuffer, &extractionData.outputBufferSize, &offset, &outputSizeProcessed, m_parentArchive->getRawAllocatorHandle(), &temporaryAllocator) != SZ_OK) {
		return nullptr;
	}

	return std::make_unique<ByteBuffer>(extractionData.outputBuffer + offset, outputSizeProcessed);
}

uint32_t SevenZipArchive::Entry::getCRC32() const {
	if(!isParentArchiveValid()) {
		return 0;
	}

	return m_parentArchive->getRawArchiveHandle()->CRCs.Vals[m_index];
}

bool SevenZipArchive::Entry::writeTo(const std::string & directoryPath, bool overwrite) const {
	std::unique_ptr<ByteBuffer> data(getData());
	std::string path(getPath());

	if(data == nullptr) {
		spdlog::error("Failed to obtain 7-Zip entry file data when writing entry '{}' to directory: '{}'.", path, directoryPath);
		return false;
	}

	return data->writeTo(Utilities::joinPaths(directoryPath, path), overwrite);
}

SevenZipArchive * SevenZipArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

void SevenZipArchive::Entry::clearParentArchive() {
	m_parentArchive = nullptr;
}

bool SevenZipArchive::Entry::isParentArchiveValid() const {
	return m_parentArchive != nullptr;
}

#include "SevenZipArchive.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <SevenZip/C/7z.h>

#include <spdlog/spdlog.h>

SevenZipArchive::Entry::Entry(uint64_t index, SevenZipArchive * parentArchive)
	: m_index(index)
	, m_parentArchive(parentArchive) { }

SevenZipArchive::Entry::~Entry() { }

bool SevenZipArchive::Entry::isFile() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	return !SzArEx_IsDir(m_parentArchive->getRawArchiveHandle(), m_index);
}

bool SevenZipArchive::Entry::isDirectory() const {
	if(!isParentArchiveValid()) {
		return false;
	}

	return SzArEx_IsDir(m_parentArchive->getRawArchiveHandle(), m_index);
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

uint64_t SevenZipArchive::Entry::getIndex() const {
	return m_index;
}

bool SevenZipArchive::Entry::hasComment() const {
	return false;
}

std::string SevenZipArchive::Entry::getComment() const {
	return "";
}

std::chrono::time_point<std::chrono::system_clock> SevenZipArchive::Entry::getDate() const {
	const CSzArEx * archiveHandle = m_parentArchive->getRawArchiveHandle();

	if(!SzBitWithVals_Check(&archiveHandle->MTime, m_index)) {
		return std::chrono::system_clock::from_time_t(time_t{0});
	}

	return SevenZipArchive::getTimePointFromNTFSFileTime(archiveHandle->MTime.Vals[m_index]);
}

uint64_t SevenZipArchive::Entry::getCompressedSize() const {
	// Note: 7-Zip does not report the compressed size of archive file entries
	return 0;
}

uint64_t SevenZipArchive::Entry::getUncompressedSize() const {
	if(!isParentArchiveValid()) {
		return 0;
	}

	return SzArEx_GetFileSize(m_parentArchive->getRawArchiveHandle(), m_index);
}

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

bool SevenZipArchive::Entry::writeToFile(const std::string & filePath, bool overwrite) {
	std::unique_ptr<ByteBuffer> data(getData());

	if(data == nullptr) {
		spdlog::error("Failed to obtain 7-Zip entry file data when writing entry to file: '{}'.", filePath);
		return false;
	}

	return data->writeTo(filePath, overwrite);
}

Archive * SevenZipArchive::Entry::getParentArchive() const {
	return m_parentArchive;
}

bool SevenZipArchive::Entry::setParentArchive(Archive * archive) {
	if(archive == nullptr) {
		m_parentArchive = nullptr;
		return true;
	}

	SevenZipArchive * sevenZipArchive = dynamic_cast<SevenZipArchive *>(archive);

	if(sevenZipArchive == nullptr) {
		return false;
	}

	m_parentArchive = sevenZipArchive;

	return true;
}

#include "FileUtilities.h"

#include "StringUtilities.h"

#include <ByteBuffer.h>

#include <cryptopp/cryptlib.h>
#include <cryptopp/sha.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>

#if _WIN32

const char Utilities::newLine[] = "\r\n";

#else

const char Utilities::newLine[] = "\n";

#endif // _WIN32

size_t getFileExtensionSeparatorIndex(std::string_view filePath) {
	size_t pathSeparatorIndex = filePath.find_last_of("/\\");

	if(pathSeparatorIndex == std::numeric_limits<size_t>::max()) {
		pathSeparatorIndex = 0;
	}

	return filePath.find_first_of(".", pathSeparatorIndex + 1);
}

bool Utilities::startsWithPathSeparator(std::string_view filePath) {
	if(filePath.empty()) {
		return false;
	}

	char startCharacter = filePath[0];

	return startCharacter == '/' || startCharacter == '\\';
}

bool Utilities::endsWithPathSeparator(std::string_view filePath) {
	if(filePath.empty()) {
		return false;
	}

	char endCharacter = filePath[filePath.length() - 1];

	return endCharacter == '/' || endCharacter == '\\';
}

std::string_view Utilities::getFileName(std::string_view filePath) {
	size_t index = filePath.find_last_of("/\\");

	if(index == std::string::npos) {
		return filePath;
	}

	return std::string_view(filePath.data() + index + 1, filePath.length() - (index + 1));
}

std::string_view Utilities::getFilePath(std::string_view filePath) {
	size_t index = filePath.find_last_of("/\\");

	if(index == std::string::npos) {
		return filePath;
	}

	return std::string_view(filePath.data(), index);
}

std::string_view Utilities::getBasePath(std::string_view filePath) {
	size_t index = filePath.find_last_of("/\\");

	if(index == std::string::npos) {
		return Utilities::emptyString;
	}

	return std::string_view(filePath.data(), index);
}

std::string_view Utilities::trimLeadingPathSeparator(std::string_view filePath) {
	size_t pathStartIndex = filePath.find_first_not_of("/\\");

	if(pathStartIndex == std::string::npos) {
		return filePath;
	}

	return std::string_view(filePath.data() + pathStartIndex, filePath.length() - pathStartIndex);
}

std::string_view Utilities::trimTrailingPathSeparator(std::string_view filePath) {
	size_t pathEndIndex = filePath.find_last_not_of("/\\");

	if(pathEndIndex == std::string::npos) {
		return filePath;
	}

	return std::string_view(filePath.data(), pathEndIndex + 1);
}

std::string_view Utilities::trimPathSeparators(std::string_view filePath) {
	size_t pathStartIndex = filePath.find_first_not_of("/\\");
	size_t pathEndIndex = filePath.find_last_not_of("/\\");

	if(pathStartIndex == std::string::npos || pathEndIndex == std::string::npos) {
		return filePath;
	}

	return std::string_view(filePath.data() + pathStartIndex, pathEndIndex - pathStartIndex + 1);
}

std::string Utilities::addLeadingPathSeparator(std::string_view filePath, char pathSeparator) {
	if(filePath.empty()) {
		return {};
	}

	if(Utilities::startsWithPathSeparator(filePath)) {
		return std::string(filePath);
	}

	return pathSeparator + std::string(filePath);
}

std::string Utilities::addTrailingPathSeparator(std::string_view filePath, char pathSeparator) {
	if(filePath.empty()) {
		return {};
	}

	if(Utilities::endsWithPathSeparator(filePath)) {
		return std::string(filePath);
	}

	return std::string(filePath) + pathSeparator;
}

std::string_view Utilities::getFileExtension(std::string_view filePath) {
	size_t index = getFileExtensionSeparatorIndex(filePath);

	if(index == std::string::npos) {
		return std::string_view();
	}

	return std::string_view(filePath.data() + index + 1, filePath.length() - (index + 1));
}

std::string_view Utilities::getFileNameNoExtension(std::string_view filePath) {
	size_t index = getFileExtensionSeparatorIndex(filePath);

	if(index == std::string::npos) {
		return filePath;
	}

	return std::string_view(filePath.data(), index);
}

bool Utilities::hasFileExtension(std::string_view filePath, std::string_view extension, bool caseSensitive) {
	if(filePath.empty() || extension.empty()) {
		return false;
	}

	return Utilities::areStringsEqual(Utilities::getFileExtension(filePath), extension, caseSensitive);
}

std::string Utilities::replaceFileExtension(std::string_view filePath, std::string_view extension) {
	if(Utilities::getFileExtension(filePath).empty()) {
		return std::string(filePath);
	}

	return std::string(Utilities::getFileNameNoExtension(filePath)) + "." + std::string(extension);
}

std::string Utilities::reverseFileExtension(std::string_view filePath) {
	return Utilities::replaceFileExtension(filePath, Utilities::reverseString(Utilities::getFileExtension(filePath)));
}

std::string Utilities::truncateFileName(std::string_view filePath, size_t maxLength) {
	if(filePath.empty()) {
		return std::string();
	}

	std::string basePath;

	if(filePath.find_first_of("/\\") != std::string::npos) {
		basePath = Utilities::getFilePath(filePath);
	}

	std::string fileName(Utilities::getFileName(filePath));

	if(fileName.empty()) {
		return basePath;
	}

	if(maxLength == 0) {
		return basePath;
	}

	if(fileName.length() <= maxLength) {
		return Utilities::joinPaths(basePath, fileName);
	}

	size_t index = getFileExtensionSeparatorIndex(filePath);

	if(index == std::string::npos) {
		return Utilities::joinPaths(basePath, fileName.substr(0, maxLength));
	}

	std::string baseFileName(fileName.substr(0, index));
	std::string extension(fileName.substr(index + 1, fileName.length() - (index + 1)));

	if(maxLength - (extension.length() + (extension.length() > 0 ? 1 : 0)) < 1) {
		return fileName.substr(0, maxLength);
	}

	return fileName.substr(0, maxLength - extension.length() - (extension.length() > 0 ? 1 : 0)) + (extension.length() > 0 ? "." + extension : "");
}

std::vector<std::string_view> Utilities::splitPath(std::string_view path) {
	if(path.empty()) {
		return {};
	}

	size_t previousPathSeparatorIndex = std::string::npos;
	size_t currentPathSeparatorIndex = 0;
	std::string_view pathPart;
	std::vector<std::string_view> pathParts;

	while(true) {
		currentPathSeparatorIndex = path.find_first_of("/\\", previousPathSeparatorIndex == std::string::npos ? 0 : previousPathSeparatorIndex + 1);

		if(currentPathSeparatorIndex == std::string::npos) {
			pathPart = std::string_view(path.data() + (previousPathSeparatorIndex == std::string::npos ? 0 : previousPathSeparatorIndex), path.length() - (previousPathSeparatorIndex == std::string::npos ? 0 : previousPathSeparatorIndex));

			size_t pathPartEndIndex = pathPart.find_last_not_of("/\\");

			if(pathPartEndIndex == std::string::npos) {
				break;
			}

			pathPart = std::string_view(pathPart.data(), pathPartEndIndex);

			if(!pathPart.empty()) {
				pathParts.emplace_back(pathPart);
			}

			break;
		}

		pathPart = std::string_view(path.data() + (previousPathSeparatorIndex == std::string::npos ? 0 : previousPathSeparatorIndex + 1), currentPathSeparatorIndex - (previousPathSeparatorIndex == std::string::npos ? 0 : previousPathSeparatorIndex + 1));

		if(!pathPart.empty()) {
			pathParts.emplace_back(pathPart);
		}

		previousPathSeparatorIndex = currentPathSeparatorIndex;
	}

	return pathParts;
}

std::string Utilities::getSafeDirectoryName(std::string_view value) {
	static const std::map<char, std::string> INVALID_CHARACTER_SUBSTITUTIONS = {
		{ '<', "" },
		{ '>', "" },
		{ '"', "'" },
		{ ':', " -" },
		{ '/', "" },
		{ '\\', "" },
		{ '|', "" },
		{ '?', "" },
		{ '*', "" }
	};

	std::string safeDirectoryName(value);

	for(std::pair<char, std::string> invalidCharacterSubtitution : INVALID_CHARACTER_SUBSTITUTIONS) {
		while(true) {
			size_t invalidCharacterIndex = safeDirectoryName.find(invalidCharacterSubtitution.first);

			if(invalidCharacterIndex == std::string::npos) {
				break;
			}

			safeDirectoryName.replace(invalidCharacterIndex, 1, invalidCharacterSubtitution.second);
		}
	}

	size_t lastPeriodIndex = std::string::npos;

	if(!safeDirectoryName.empty() && safeDirectoryName[safeDirectoryName.length() - 1] == '.') {
		lastPeriodIndex = safeDirectoryName.length() - 1;

		for(size_t i = safeDirectoryName.length() - 2; i >= 0; i--) {
			if(safeDirectoryName[i] != '.') {
				break;
			}

			lastPeriodIndex = i;
		}
	}

	if(lastPeriodIndex == std::string::npos) {
		return safeDirectoryName;
	}

	return safeDirectoryName.substr(0, lastPeriodIndex);
}

std::string Utilities::getFileMD5Hash(const std::string & filePath, ByteBuffer::HashFormat hashFormat) {
	return getFileHash(filePath, ByteBuffer::HashType::MD5, hashFormat);
}

std::string Utilities::getFileSHA1Hash(const std::string & filePath, ByteBuffer::HashFormat hashFormat) {
	return getFileHash(filePath, ByteBuffer::HashType::SHA1, hashFormat);
}

std::string Utilities::getFileSHA256Hash(const std::string & filePath, ByteBuffer::HashFormat hashFormat) {
	return getFileHash(filePath, ByteBuffer::HashType::SHA256, hashFormat);
}

std::string Utilities::getFileSHA512Hash(const std::string & filePath, ByteBuffer::HashFormat hashFormat) {
	return getFileHash(filePath, ByteBuffer::HashType::SHA512, hashFormat);
}

std::string Utilities::getFileHash(const std::string & filePath, ByteBuffer::HashType hashType, ByteBuffer::HashFormat hashFormat) {
	std::ifstream inputFileStream(filePath, std::ios::binary | std::ios::ate);

	if(!inputFileStream.is_open()) {
		spdlog::error("Failed to open file '{}' for hashing.", filePath);
		return {};
	}

	std::streampos fileEnd = inputFileStream.tellg();
	inputFileStream.seekg(0, std::ios::beg);
	size_t fileSize = std::size_t(fileEnd - inputFileStream.tellg());

	if(fileSize == 0) {
		spdlog::debug("File '{}' is empty, skipping hashing.", filePath);
		return {};
	}

	ByteBuffer fileData(fileSize);

	if(!inputFileStream.read(reinterpret_cast<char *>(fileData.getRawData()), fileData.getSize())) {
		spdlog::error("Failed to read file '{}', skipping hashing.", filePath);
		return {};
	}

	return fileData.getHash(hashType, hashFormat);
}

void Utilities::createDirectoryStructureForFilePath(const std::string & filePath, std::error_code & errorCode) {
	if(filePath.find_first_of("/\\") == std::string::npos) {
		return;
	}

	std::filesystem::path baseFilePath(Utilities::getFilePath(filePath));

	if(!baseFilePath.empty() && !std::filesystem::is_directory(std::filesystem::path(baseFilePath))) {
		std::filesystem::create_directories(baseFilePath, errorCode);

		if(errorCode) {
			return;
		}
	}
}

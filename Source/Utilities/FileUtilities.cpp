#include "FileUtilities.h"

#include "StringUtilities.h"

#include <ByteBuffer.h>

#include <cryptopp/cryptlib.h>
#include <cryptopp/sha.h>
#include <fmt/core.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

#if _WIN32

const char Utilities::newLine[] = "\r\n";

#else

const char Utilities::newLine[] = "\n";

#endif // _WIN32

std::string Utilities::getFileName(const std::string & filePath) {
	size_t index = std::string(filePath).find_last_of('/\\');

	if(index == std::string::npos) {
		return filePath;
	}

	return filePath.substr(index + 1, filePath.length() - (index + 1));
}


std::string Utilities::getFilePath(const std::string & filePath) {
	size_t index = std::string(filePath).find_last_of('/\\');

	if(index == std::string::npos) {
		return filePath;
	}

	return filePath.substr(0, index);
}

std::string Utilities::getFileExtension(const std::string & filePath) {
	size_t index = std::string(filePath).find_last_of('.');

	if(index == std::string::npos) {
		return std::string();
	}

	return filePath.substr(index + 1, filePath.length() - (index + 1));
}

std::string Utilities::getFileNameNoExtension(const std::string & filePath) {
	size_t index = std::string(filePath).find_last_of('.');

	if(index == std::string::npos) {
		return filePath;
	}

	return filePath.substr(0, index);
}

bool Utilities::hasFileExtension(const std::string & filePath, const std::string & extension) {
	if(filePath.empty() || extension.empty()) {
		return false;
	}

	return Utilities::compareStringsIgnoreCase(Utilities::getFileExtension(filePath), extension) == 0;
}

std::string Utilities::replaceFileExtension(const std::string & filePath, const std::string & extension) {
	if(Utilities::getFileExtension(filePath).empty()) {
		return filePath;
	}

	return Utilities::getFileNameNoExtension(filePath) + "." + extension;
}

std::string Utilities::reverseFileExtension(const std::string & filePath) {
	return Utilities::replaceFileExtension(filePath, Utilities::reverseString(Utilities::getFileExtension(filePath)));
}

std::string Utilities::truncateFileName(const std::string & filePath, size_t maxLength) {
	if(filePath.empty()) {
		return std::string();
	}

	std::string basePath(Utilities::getFilePath(filePath));
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

	size_t index = fileName.find_last_of(".");

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

std::string Utilities::joinPaths(const std::string & leftPath, const std::string & rightPath, char separator) {
	static const std::regex leftPathFormatRegExp("[ \t]*[/\\\\]+[ \t]*$");
	static const std::regex rightPathFormatRegExp("^[ \t]*[/\\\\]+[ \t]*");
	static const std::regex separatorRegExp("[/\\\\]");

	std::string formattedLeftPath;
	std::string formattedRightPath;
	std::regex_replace(std::back_inserter(formattedLeftPath), leftPath.begin(), leftPath.end(), leftPathFormatRegExp, "");
	std::regex_replace(std::back_inserter(formattedRightPath), rightPath.begin(), rightPath.end(), rightPathFormatRegExp, "");
	std::stringstream joinedPathsStream;

	if(!formattedLeftPath.empty()) {
		joinedPathsStream << formattedLeftPath;

		if(!formattedRightPath.empty()) {
			joinedPathsStream << "/";
		}
	}

	if(!formattedRightPath.empty()) {
		joinedPathsStream << formattedRightPath;
	}

	std::string joinedPaths(joinedPathsStream.str());
	std::string formattedJoinedPaths;
	std::regex_replace(std::back_inserter(formattedJoinedPaths), joinedPaths.begin(), joinedPaths.end(), separatorRegExp, std::string(1, separator));

	return formattedJoinedPaths;
}

std::string Utilities::getSafeDirectoryName(const std::string & value) {
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

std::optional<std::string> Utilities::getFileSHA1Hash(const std::string & filePath) {
	std::ifstream inputFileStream(filePath, std::ios::binary | std::ios::ate);

	if(!inputFileStream.is_open()) {
		fmt::print("Failed to open file '{}' for hashing.\n", filePath);
		return {};
	}

	std::streampos fileEnd = inputFileStream.tellg();
	inputFileStream.seekg(0, std::ios::beg);
	size_t fileSize = std::size_t(fileEnd - inputFileStream.tellg());

	if(fileSize == 0) {
		fmt::print("File '{}' is empty, skipping hashing.\n", filePath);
		return {};
	}

	ByteBuffer fileData(fileSize);

	if(!inputFileStream.read(reinterpret_cast<char *>(fileData.getRawData()), fileData.getSize())) {
		fmt::print("Failed to read file '{}' is empty, skipping hashing.\n", filePath);
		return {};
	}

	return fileData.getSHA1();
}

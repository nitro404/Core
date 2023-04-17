#ifndef _FILE_UTILITIES_H_
#define _FILE_UTILITIES_H_

#include "ByteBuffer.h"

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace Utilities {

	extern const char newLine[];

	bool startsWithPathSeparator(std::string_view filePath);
	bool endsWithPathSeparator(std::string_view filePath);
	std::string_view getFileName(std::string_view filePath);
	std::string_view getFilePath(std::string_view filePath);
	std::string_view getBasePath(std::string_view filePath);
	std::string getAbsoluteFilePath(std::string_view filePath, std::string_view defaultPath = {});
	std::string_view trimLeadingPathSeparator(std::string_view filePath);
	std::string_view trimTrailingPathSeparator(std::string_view filePath);
	std::string_view trimPathSeparators(std::string_view filePath);
	std::string addLeadingPathSeparator(std::string_view filePath, char pathSeparator = '/');
	std::string addTrailingPathSeparator(std::string_view filePath, char pathSeparator = '/');
	std::string_view getFileExtension(std::string_view filePath, bool useLastPeriod = true);
	std::string_view getFileNameNoExtension(std::string_view filePath, bool useLastPeriod = true);
	bool hasFileExtension(std::string_view filePath, std::string_view extension, bool caseSensitive = false);
	std::string replaceFileExtension(std::string_view filePath, std::string_view extension);
	std::string reverseFileExtension(std::string_view filePath);
	std::string truncateFileName(std::string_view filePath, size_t maxLength);
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::string joinPaths(Arguments &&... arguments);
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::string joinPaths(char separator, Arguments &&... arguments);
	std::vector<std::string_view> splitPath(std::string_view path);
	std::string getSafeDirectoryName(std::string_view value);
	std::string getFileMD5Hash(const std::string & filePath, ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT);
	std::string getFileSHA1Hash(const std::string & filePath, ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT);
	std::string getFileSHA256Hash(const std::string & filePath, ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT);
	std::string getFileSHA512Hash(const std::string & filePath, ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT);
	std::string getFileHash(const std::string & filePath, ByteBuffer::HashType hashType, ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT);
	void createDirectoryStructureForFilePath(const std::string & filePath, std::error_code & errorCode);
	bool areSymlinksSupported();

}

template <typename ...Arguments, typename>
std::string Utilities::joinPaths(Arguments &&... arguments) {
	return Utilities::joinPaths('/', arguments...);
}

template <typename ...Arguments, typename>
std::string Utilities::joinPaths(char separator, Arguments &&... arguments) {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};
	std::stringstream joinedPaths;
	bool skipSeparator = false;

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		if(joinedPaths.tellp() == 0) {
			size_t trimmedPartStartIndex = unpackedArguments[i].find_first_not_of(" \t");
			size_t trimmedPartEndIndex = unpackedArguments[i].find_last_not_of(" \t");

			if(trimmedPartStartIndex == std::string::npos || trimmedPartEndIndex == std::string::npos) {
				continue;
			}

			std::string_view trimmedPart(unpackedArguments[i].data() + trimmedPartStartIndex, trimmedPartEndIndex - trimmedPartStartIndex + 1);

			if(trimmedPart.length() == 1 && trimmedPart[0] == '/' && separator == '/') {
				skipSeparator = true;
				joinedPaths << '/';
				continue;
			}
		}

		size_t strippedPartStartIndex = 0;

		if(joinedPaths.tellp() == 0 && separator == '/') {
			strippedPartStartIndex = unpackedArguments[i].find_first_not_of(" \t\\");
		}
		else {
			strippedPartStartIndex = unpackedArguments[i].find_first_not_of(" \t/\\");
		}

		size_t strippedPartEndIndex = unpackedArguments[i].find_last_not_of(" \t/\\");

		if(strippedPartStartIndex == std::string::npos || strippedPartEndIndex == std::string::npos) {
			continue;
		}

		std::string_view strippedPart(unpackedArguments[i].data() + strippedPartStartIndex, strippedPartEndIndex - strippedPartStartIndex + 1);

		if(joinedPaths.tellp() != 0 && !strippedPart.empty()) {
			if(skipSeparator) {
				skipSeparator = false;
			}
			else {
				joinedPaths << separator;
			}
		}

		joinedPaths << strippedPart;
	}

	return joinedPaths.str();
}

#endif // _FILE_UTILITIES_H_

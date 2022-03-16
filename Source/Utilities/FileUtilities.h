#ifndef _FILE_UTILITIES_H_
#define _FILE_UTILITIES_H_

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>

namespace Utilities {

	extern const char newLine[];

	std::string getFileName(const std::string & filePath);
	std::string getFilePath(const std::string & filePath);
	std::string getFileExtension(const std::string & filePath);
	std::string getFileNameNoExtension(const std::string & filePath);
	bool hasFileExtension(const std::string & filePath, const std::string & extension);
	std::string replaceFileExtension(const std::string & filePath, const std::string & extension);
	std::string reverseFileExtension(const std::string & filePath);
	std::string truncateFileName(const std::string & filePath, size_t maxLength);
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::string joinPaths(Arguments &&... arguments);
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::string joinPaths(char separator, Arguments &&... arguments);
	std::string getSafeDirectoryName(const std::string &value);
	std::optional<std::string> getFileSHA1Hash(const std::string & filePath);

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

#ifndef _FILE_UTILITIES_H_
#define _FILE_UTILITIES_H_

#include <optional>
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
	std::string joinPaths(const std::string & leftPath, const std::string & rightPath, char separator = '/');
	std::string getSafeDirectoryName(const std::string &value);
	std::optional<std::string> getFileSHA1Hash(const std::string & filePath);

}

#endif // _FILE_UTILITIES_H_

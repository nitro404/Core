#ifndef _STRING_UTILITIES_H_
#define _STRING_UTILITIES_H_

#include <cstdint>
#include <string>

namespace Utilities {

	unsigned int stringLength(const char * s);
	const char * toString(int value);
	const char * toString(double value);
	bool copyString(char * destination, int size, const char * source);
	char * copyString(const char * data);
	char * trimCopyString(const char * data);
	std::string trimString(const std::string & data, bool trimWhiteSpace = true, bool trimNewLines = true);
	char * substring(const char * data, int start, int end);
	std::string substring(const std::string & data, int start, int end);
	std::string wideStringToString(const std::wstring & wideString);
	std::wstring stringToWideString(const std::string & string);
	int compareStrings(const std::string & s1, const std::string & s2, bool caseSensitive = true);
	int compareStringsIgnoreCase(const std::string & s1, const std::string & s2);
	bool isComment(const std::string & data, const std::string & comment = "//");
	std::string getVariableID(const std::string & data);
	std::string getVariableValue(const std::string & data);
	bool parseBoolean(const std::string & data, bool * valid = nullptr);
	int parseInteger(const std::string & data, bool * valid = nullptr);
	float parseFloat(const std::string & data, bool * valid = nullptr);

}

#endif // _STRING_UTILITIES_H_

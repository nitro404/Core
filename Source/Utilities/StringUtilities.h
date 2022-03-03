#ifndef _STRING_UTILITIES_H_
#define _STRING_UTILITIES_H_

#include <cstdint>
#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace Utilities {

	const std::string emptyString;
	bool isAlphaNumericUpperCase(char c);
	bool isAlphaNumericLowerCase(char c);
	size_t stringLength(const char * s);
	bool copyString(char * destination, size_t size, const char * source);
	char * copyString(const char * data);
	char * trimCopyString(const char * data);
	std::string trimString(const std::string & data, bool trimWhiteSpace = true, bool trimNewLines = true);
	char * substring(const char * data, size_t start, size_t length);
	std::string reverseString(const std::string & data);
	std::string replaceAll(const std::string & value, const std::string & match, const std::string & replacement);
	std::vector<std::string> regularExpressionStringSplit(const std::string & data, const std::string & regularExpression, std::regex_constants::match_flag_type matchFlags = std::regex_constants::match_default);
	std::vector<std::string> regularExpressionStringSplit(const std::string & data, const std::regex & regularExpression, std::regex_constants::match_flag_type matchFlags = std::regex_constants::match_default);
	std::string wideStringToString(const std::wstring & wideString);
	std::wstring stringToWideString(const std::string & string);
	std::string toCamelCase(const std::string & string);
	std::string toCapitalCase(const std::string & string);
	std::string toConstantCase(const std::string & string);
	std::string toDotCase(const std::string & string);
	std::string toHeaderCase(const std::string & string);
	std::string toLowerCase(const std::string & string);
	std::string toParamCase(const std::string & string);
	std::string toPascalCase(const std::string & string);
	std::string toPathCase(const std::string & string);
	std::string toSnakeCase(const std::string & string);
	std::string toSpongeCase(const std::string & string);
	std::string toUpperCase(const std::string & string);
	std::string swapCase(const std::string & string);
	int32_t compareStrings(const std::string & s1, const std::string & s2, bool caseSensitive = true);
	int32_t compareStringsIgnoreCase(const std::string & s1, const std::string & s2);
	bool isComment(const std::string & data, const std::string & comment = "//");
	bool isURL(const std::string & data);
	bool isEmailAddress(const std::string & data);
	bool isIPV4Address(const std::string & data);
	bool isIPV6Address(const std::string & data);
	bool isMACAddress(const std::string & data);
	std::string getVariableID(const std::string & data);
	std::string getVariableValue(const std::string & data);
	bool isValidIntegerNumber(const std::string & data);
	bool isValidWholeNumber(const std::string & data);
	bool isValidRealNumber(const std::string & data);
	bool parseBoolean(const std::string & data, bool * error);
	std::optional<bool> parseBoolean(const std::string & data);
	int8_t parseByte(const std::string & data, bool * error);
	std::optional<int8_t> parseByte(const std::string & data);
	uint8_t parseUnsignedByte(const std::string & data, bool * error);
	std::optional<uint8_t> parseUnsignedByte(const std::string & data);
	int16_t parseShort(const std::string & data, bool * error);
	std::optional<int16_t> parseShort(const std::string & data);
	uint16_t parseUnsignedShort(const std::string & data, bool * error);
	std::optional<uint16_t> parseUnsignedShort(const std::string & data);
	int32_t parseInteger(const std::string & data, bool * error);
	std::optional<int32_t> parseInteger(const std::string & data);
	uint32_t parseUnsignedInteger(const std::string & data, bool * error);
	std::optional<uint32_t> parseUnsignedInteger(const std::string & data);
	int64_t parseLong(const std::string & data, bool * error);
	std::optional<int64_t> parseLong(const std::string & data);
	uint64_t parseUnsignedLong(const std::string & data, bool * error);
	std::optional<uint64_t> parseUnsignedLong(const std::string & data);
	float parseFloat(const std::string & data, bool * error);
	std::optional<float> parseFloat(const std::string & data);
	double parseDouble(const std::string & data, bool * error);
	std::optional<double> parseDouble(const std::string & data);
	std::optional<std::vector<std::string>> parseVersion(const std::string & value, bool trimTrailingZeroes = false);
	std::optional<int8_t> compareVersions(const std::string & v1, const std::string & v2, bool caseSensitive = false);

}

#endif // _STRING_UTILITIES_H_

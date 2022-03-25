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
	std::string trimString(std::string_view data, bool trimWhiteSpace = true, bool trimNewLines = true);
	char * substring(const char * data, size_t start, size_t length);
	std::string reverseString(std::string_view data);
	std::string replaceAll(std::string_view value, std::string_view match, std::string_view replacement);
	std::vector<std::string> regularExpressionStringSplit(const std::string & data, const std::string & regularExpression, std::regex_constants::match_flag_type matchFlags = std::regex_constants::match_default);
	std::vector<std::string> regularExpressionStringSplit(const std::string & data, const std::regex & regularExpression, std::regex_constants::match_flag_type matchFlags = std::regex_constants::match_default);
	std::string wideStringToString(const std::wstring & wideString);
	std::wstring stringToWideString(const std::string & string);
	std::string toCamelCase(std::string_view string);
	std::string toCapitalCase(std::string_view string);
	std::string toConstantCase(std::string_view string);
	std::string toDotCase(std::string_view string);
	std::string toHeaderCase(std::string_view string);
	std::string toLowerCase(std::string_view string);
	std::string toParamCase(std::string_view string);
	std::string toPascalCase(std::string_view string);
	std::string toPathCase(std::string_view string);
	std::string toSnakeCase(std::string_view string);
	std::string toSpongeCase(std::string_view string);
	std::string toUpperCase(std::string_view string);
	std::string swapCase(std::string_view string);
	bool areStringsEqual(std::string_view s1, std::string_view s2, bool caseSensitive = true);
	int32_t compareStrings(std::string_view s1, std::string_view s2, bool caseSensitive = true);
	bool areStringsEqualIgnoreCase(std::string_view s1, std::string_view s2);
	int32_t compareStringsIgnoreCase(std::string_view s1, std::string_view s2);
	bool isComment(std::string_view data, std::string_view comment = "//");
	bool isURL(const std::string & data);
	bool isEmailAddress(const std::string & data);
	bool isIPV4Address(const std::string & data);
	bool isIPV6Address(const std::string & data);
	bool isMACAddress(const std::string & data);
	std::string getVariableID(std::string_view data);
	std::string getVariableValue(std::string_view data);
	bool isValidIntegerNumber(const std::string & data);
	bool isValidWholeNumber(const std::string & data);
	bool isValidRealNumber(const std::string & data);
	bool parseBoolean(std::string_view data, bool * error);
	std::optional<bool> parseBoolean(std::string_view data);
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

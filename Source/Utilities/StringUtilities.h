#ifndef _STRING_UTILITIES_H_
#define _STRING_UTILITIES_H_

#include "Endianness.h"

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
	std::string trimString(std::string_view data, bool trimWhiteSpace = true, bool trimNewLines = true);
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
	bool areStringsEqual(std::string_view s1, std::string_view s2, bool caseSensitive = true, bool ignoreCarriageReturn = false);
	int32_t compareStrings(std::string_view s1, std::string_view s2, bool caseSensitive = true, bool ignoreCarriageReturn = false);
	bool areStringsEqualIgnoreCase(std::string_view s1, std::string_view s2, bool ignoreCarriageReturn = false);
	int32_t compareStringsIgnoreCase(std::string_view s1, std::string_view s2, bool ignoreCarriageReturn = false);
	bool startsWith(std::string_view value, std::string_view suffix, bool caseSensitive = true);
	bool endsWith(std::string_view value, std::string_view suffix, bool caseSensitive = true);
	bool isComment(std::string_view data, std::string_view comment = "//");
	bool isURL(const std::string & data);
	bool isEmailAddress(const std::string & data);
	bool isIPV4Address(const std::string & data);
	bool isIPV6Address(const std::string & data);
	bool isMACAddress(const std::string & data);
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
	uint16_t toShortString(std::string_view value, char paddingCharacter = ' ');
	uint32_t toIntegerString(std::string_view value, char paddingCharacter = ' ');
	uint64_t toLongString(std::string_view value, char paddingCharacter = ' ');
	uint16_t toShortString(std::string_view value, Endianness endianness);
	uint32_t toIntegerString(std::string_view value, Endianness endianness);
	uint64_t toLongString(std::string_view value, Endianness endianness);
	uint16_t toShortString(std::string_view value, char paddingCharacter, Endianness endianness);
	uint32_t toIntegerString(std::string_view value, char paddingCharacter, Endianness endianness);
	uint64_t toLongString(std::string_view value, char paddingCharacter, Endianness endianness);
	std::string fromShortString(uint16_t value);
	std::string fromIntegerString(uint32_t value);
	std::string fromLongString(uint64_t value);
	std::string fromShortString(uint16_t value, Endianness endianness);
	std::string fromIntegerString(uint32_t value, Endianness endianness);
	std::string fromLongString(uint64_t value, Endianness endianness);
	std::optional<std::vector<std::string>> parseVersion(const std::string & value, bool trimTrailingZeroes = false);
	std::optional<int8_t> compareVersions(const std::string & v1, const std::string & v2, bool caseSensitive = false);
	std::string_view readLine(std::string_view data, size_t & offset);

}

#endif // _STRING_UTILITIES_H_

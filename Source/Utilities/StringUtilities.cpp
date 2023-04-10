#include "StringUtilities.h"

#include "Utilities.h"

#include <algorithm>
#include <codecvt>
#include <functional>
#include <locale>
#include <regex>
#include <sstream>
#include <string>

static std::string transformStringCase(std::string_view string, std::string_view delimiter, std::function<std::string(std::string_view, size_t, size_t)> tokenTransformation) {
	if(string.length() <= 1) {
		return std::string(string);
	}

	size_t i = string.length() - 1;
	std::string tokenizedString(string);

	while(true) {
		if(i < tokenizedString.length() - 2 && Utilities::isAlphaNumericLowerCase(tokenizedString[i]) && std::isupper(tokenizedString[i + 1])) {
			tokenizedString.insert(tokenizedString.begin() + i + 1, '\0');
		}

		if(tokenizedString.length() >= 3 && i < tokenizedString.length() - 3 && std::isupper(tokenizedString[i]) && std::isupper(tokenizedString[i + 1]) && std::islower(tokenizedString[i + 2])) {
			tokenizedString.insert(tokenizedString.begin() + i + 1, '\0');
		}

		if(i == 0) {
			break;
		}

		i--;
	}

	std::string sanitizedString(tokenizedString.length(), '\0');

	std::transform(
		tokenizedString.begin(),
		tokenizedString.end(),
		sanitizedString.begin(),
		[] (char c) {
			if(!std::isalnum(c)) {
				return '\0';
			}

			return c;
		}
	);

	tokenizedString.clear();

	size_t startIndex = 0;
	size_t endIndex = sanitizedString.length() - 1;

	while(sanitizedString[startIndex] == '\0') {
		if(startIndex == endIndex) {
			break;
		}

		startIndex++;
	}

	while(sanitizedString[endIndex] == '\0') {
		if(endIndex == 0) {
			break;
		}

		endIndex--;
	}

	if(startIndex == sanitizedString.length() - 1 || endIndex == 0) {
		return "";
	}

	std::string trimmedString(sanitizedString.substr(startIndex, endIndex - startIndex + 1));

	sanitizedString.clear();

	startIndex = 0;
	std::vector<std::string> tokens;

	while(true) {
		endIndex = startIndex;

		while(trimmedString[endIndex] != '\0') {
			if(endIndex == trimmedString.length() - 1) {
				break;
			}

			endIndex++;
		}

		if(endIndex == trimmedString.length() - 1) {
			tokens.push_back(trimmedString.substr(startIndex, trimmedString.length()));
			break;
		}

		tokens.push_back(trimmedString.substr(startIndex, endIndex - startIndex));

		startIndex = endIndex + 1;
	}

	trimmedString.clear();

	std::stringstream formattedString;

	for(size_t i = 0; i < tokens.size(); i++) {
		const std::string & token = tokens[i];

		if(formattedString.tellp() != 0) {
			formattedString << delimiter;
		}

		formattedString << tokenTransformation(token, i, tokens.size());
	}

	return formattedString.str();
}

extern const std::string Utilities::emptyString;

bool Utilities::isAlphaNumericUpperCase(char c) {
	return std::isdigit(c) || std::isupper(c);
}

bool Utilities::isAlphaNumericLowerCase(char c) {
	return std::isdigit(c) || std::islower(c);
}

size_t Utilities::stringLength(const char * s) {
	return s == nullptr ? 0 : strlen(s);
}

bool Utilities::copyString(char * destination, size_t size, const char * source) {
	if(source == nullptr || destination == nullptr || size == 0) {
		return false;
	}

#if _WIN32
	return strcpy_s(destination, size, source) == 0;
#else
	return strcpy(destination, source) == 0;
#endif // _WIN32
}

char * Utilities::copyString(const char * data) {
	if(data == nullptr) {
		return nullptr;
	}

	size_t bufferSize = Utilities::stringLength(data) + 1;
	char * newData = new char[bufferSize];

	if(Utilities::copyString(newData, bufferSize, data)) {
		return newData;
	}

	return nullptr;
}

// trims whitespace off of the front and end of string passed into it, and returns a copy of the trimmed string
char * Utilities::trimCopyString(const char * data) {
	// verify the string
	if(data == nullptr) {
		return nullptr;
	}

	char * newData = nullptr;
	size_t length = Utilities::stringLength(data);

	if(length == 0) {
		newData = new char[1];
		*newData = '\0';
		return newData;
	}

	// find the new start and end of the string and verify that they do not overlap (0 length string)
	const char * head = data;
	const char * tail = data + (sizeof(char) * length) - 1;
	size_t startPos = 0;
	size_t endPos = length - 1;

	while((*head == ' ' || *head == '\t') && startPos <= endPos) {
		head += sizeof(char);
		startPos++;
	}

	while((*tail == ' ' || *tail == '\t') && startPos <= endPos) {
		tail -= sizeof(char);
		endPos--;
	}

	if(startPos > endPos) {
		newData = new char[1];
		*newData = '\0';

		return newData;
	}

	// copy the contents of the string from the start to the end into a new string (trim) and return the copy
	size_t newLength = endPos - startPos + 2;
	newData = new char[newLength];
	memcpy(newData, data + (startPos * sizeof(char)), newLength - 1);
	newData[newLength - 1] = '\0';

	return newData;
}

std::string Utilities::trimString(std::string_view data, bool trimWhiteSpace, bool trimNewLines) {
	if(data.empty()) {
		return std::string();
	}

	if(trimWhiteSpace == false && trimNewLines == false) {
		return std::string(data);
	}

	std::string whiteSpace;

	if(trimWhiteSpace) {
		whiteSpace = " \t";
	}

	if(trimNewLines) {
		whiteSpace.append("\n\r");
	}

	size_t start = data.find_first_not_of(whiteSpace);

	if(start == std::string::npos) {
		start = 0;
	}

	size_t end = data.find_last_not_of(whiteSpace);

	if(end == std::string::npos) {
		end = data.length() - 1;
	}

	if(start > end) {
		return std::string();
	}

	return std::string(data.data() + start, end - start + 1);
}

char * Utilities::substring(const char * data, size_t start, size_t length) {
	if(data == nullptr) {
		return nullptr;
	}

	size_t dataLength = Utilities::stringLength(data);

	if(start + length > dataLength) {
		return nullptr;
	}

	if(dataLength == 0) {
		char * newString = new char[1];
		newString[0] = '\0';

		return newString;
	}

	char * newString = new char[length];
	memcpy(newString, data + (start * sizeof(char)), length - 1);
	newString[length - 1] = '\0';

	return newString;
}

std::string Utilities::reverseString(std::string_view data) {
	std::string reversed(data);

	for(size_t i = 0; i < reversed.length() / 2; i++) {
		std::swap(reversed[i], reversed[reversed.length() - i - 1]);
	}

	return reversed;
}

std::string Utilities::replaceAll(std::string_view value, std::string_view match, std::string_view replacement) {
	std::string replacedString;
	replacedString.reserve(value.length());

	size_t lastIndex = 0;
	size_t matchIndex = std::numeric_limits<size_t>::max();

	while((matchIndex = value.find(match, lastIndex)) != std::string::npos) {
		replacedString.append(value, lastIndex, matchIndex - lastIndex);
		replacedString.append(replacement);

		lastIndex = matchIndex + match.length();
	}

	replacedString.append(value.substr(lastIndex));

	return replacedString;
}

std::vector<std::string> Utilities::regularExpressionStringSplit(const std::string & data, const std::string & regularExpression, std::regex_constants::match_flag_type matchFlags) {
	return Utilities::regularExpressionStringSplit(data, std::regex(regularExpression), matchFlags);
}

std::vector<std::string> Utilities::regularExpressionStringSplit(const std::string & data, const std::regex & regularExpression, std::regex_constants::match_flag_type matchFlags) {
	std::vector<std::string> parts;
	std::sregex_token_iterator i(data.begin(), data.end(), regularExpression, -1, matchFlags);
	std::sregex_token_iterator end;

	while(i != end) {
		parts.push_back(*i);
		++i;
	}

	return parts;
}

std::string Utilities::wideStringToString(const std::wstring & wideString) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(wideString);
}

std::wstring Utilities::stringToWideString(const std::string & string) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(string);
}

std::string Utilities::toCamelCase(std::string_view string) {
	std::string camelCase(Utilities::toPascalCase(string));

	if(camelCase.size() >= 1) {
		camelCase[0] = std::tolower(camelCase[0]);
	}

	return camelCase;
}

std::string Utilities::toCapitalCase(std::string_view string) {
	return transformStringCase(
		string,
		" ",
		[] (std::string_view token, size_t tokenIndex, size_t numberOfTokens) {
			return Utilities::toPascalCase(token);
		}
	);
}

std::string Utilities::toConstantCase(std::string_view string) {
	return transformStringCase(
		string,
		"_",
		[] (std::string_view token, size_t tokenIndex, size_t numberOfTokens) {
			return Utilities::toUpperCase(token);
		}
	);
}

std::string Utilities::toDotCase(std::string_view string) {
	return transformStringCase(
		string,
		".",
		[] (std::string_view token, size_t tokenIndex, size_t numberOfTokens) {
			return Utilities::toLowerCase(token);
		}
	);
}

std::string Utilities::toHeaderCase(std::string_view string) {
	return transformStringCase(
		string,
		"-",
		[] (std::string_view token, size_t tokenIndex, size_t numberOfTokens) {
			return Utilities::toPascalCase(token);
		}
	);
}

std::string Utilities::toLowerCase(std::string_view string) {
	std::string lower(string.length(), '\0');

	std::transform(
		string.begin(),
		string.end(),
		lower.begin(),
		[] (unsigned char c) {
			return std::tolower(c);
		}
	);

	return lower;
}

std::string Utilities::toParamCase(std::string_view string) {
	return transformStringCase(
		string,
		"-",
		[] (std::string_view token, size_t tokenIndex, size_t numberOfTokens) {
			return Utilities::toLowerCase(token);
		}
	);
}

std::string Utilities::toPascalCase(std::string_view string) {
	return transformStringCase(
		string,
		"",
		[] (std::string_view token, size_t tokenIndex, size_t numberOfTokens) {
			std::string formattedToken;

			if(token.empty()) {
				return formattedToken;
			}

			formattedToken.reserve(token.length() + 1);

			if(tokenIndex != 0 && std::isdigit(token[0])) {
				formattedToken += '_';
			}

			formattedToken += static_cast<char>(std::toupper(token[0]));

			if(token.length() > 1) {
				formattedToken += Utilities::toLowerCase(token.substr(1, token.length() - 1));
			}

			return formattedToken;
		}
	);
}

std::string Utilities::toPathCase(std::string_view string) {
	return transformStringCase(
		string,
		"/",
		[] (std::string_view token, size_t tokenIndex, size_t numberOfTokens) {
			return Utilities::toLowerCase(token);
		}
	);
}

std::string Utilities::toSnakeCase(std::string_view string) {
	return transformStringCase(
		string,
		"_",
		[] (std::string_view token, size_t tokenIndex, size_t numberOfTokens) {
			return Utilities::toLowerCase(token);
		}
	);
}

std::string Utilities::toSpongeCase(std::string_view string) {
	std::string sponge(string.length(), '\0');

	std::transform(
		string.begin(),
		string.end(),
		sponge.begin(),
		[] (unsigned char c) {
			if(Utilities::randomInteger(0, 1) == 0) {
				return std::toupper(c);
			}
			else {
				return std::tolower(c);
			}
		}
	);

	return sponge;
}

std::string Utilities::toUpperCase(std::string_view string) {
	std::string upper(string.length(), '\0');

	std::transform(
		string.begin(),
		string.end(),
		upper.begin(),
		[] (unsigned char c) {
			return std::toupper(c);
		}
	);

	return upper;
}

std::string Utilities::swapCase(std::string_view string) {
	std::string swapped(string.length(), '\0');

	std::transform(
		string.begin(),
		string.end(),
		swapped.begin(),
		[] (unsigned char c) {
			if(std::isupper(c)) {
				return std::tolower(c);
			}
			else if(std::islower(c)) {
				return std::toupper(c);
			}

			return static_cast<int32_t>(c);
		}
	);

	return swapped;
}

bool Utilities::areStringsEqual(std::string_view s1, std::string_view s2, bool caseSensitive, bool ignoreCarriageReturn) {
	if(!ignoreCarriageReturn && s1.length() != s2.length()) {
		return false;
	}

	return Utilities::compareStrings(s1, s2, caseSensitive, ignoreCarriageReturn) == 0;
}

int32_t Utilities::compareStrings(std::string_view s1, std::string_view s2, bool caseSensitive, bool ignoreCarriageReturn) {
	char a = '\0';
	char b = '\0';
	size_t indexA = 0;
	size_t indexB = 0;

	while(true) {
		if(ignoreCarriageReturn) {
			while(indexA < s1.length() && s1[indexA] == '\r') {
				indexA++;
			}

			while(indexB < s2.length() && s2[indexB] == '\r') {
				indexB++;
			}
		}

		if(indexA >= s1.length()) {
			if(indexB < s2.length()) {
				return ignoreCarriageReturn ? -1 : s1.length() - s2.length();
			}
			else {
				return 0;
			}
		}

		if(indexB >= s2.length()) {
			if(indexA < s1.length()) {
				return ignoreCarriageReturn ? 1 : s1.length() - s2.length();
			}
			else {
				return 0;
			}
		}

		if(caseSensitive) {
			a = s1[indexA];
			b = s2[indexB];
		}
		else {
			a = std::tolower(s1[indexA]);
			b = std::tolower(s2[indexB]);
		}

		indexA++;
		indexB++;

		if(a == b) {
			continue;
		}

		return a - b;
	}
}

bool Utilities::areStringsEqualIgnoreCase(std::string_view s1, std::string_view s2, bool ignoreCarriageReturn) {
	if(!ignoreCarriageReturn && s1.length() != s2.length()) {
		return false;
	}

	return Utilities::compareStrings(s1, s2, false, ignoreCarriageReturn) == 0;
}

int32_t Utilities::compareStringsIgnoreCase(std::string_view s1, std::string_view s2, bool ignoreCarriageReturn) {
	return Utilities::compareStrings(s1, s2, false, ignoreCarriageReturn);
}

bool Utilities::startsWith(std::string_view value, std::string_view suffix, bool caseSensitive) {
	if(value.length() < suffix.length()) {
		return false;
	}

	return Utilities::areStringsEqual(std::string_view(value.data(), suffix.length()), suffix, caseSensitive);
}

bool Utilities::endsWith(std::string_view value, std::string_view suffix, bool caseSensitive) {
	if(value.length() < suffix.length()) {
		return false;
	}

	return Utilities::areStringsEqual(std::string_view(value.data() + value.length() - suffix.length(), suffix.length()), suffix, caseSensitive);
}

bool Utilities::isComment(std::string_view data, std::string_view comment) {
	if(comment.empty()) {
		return false;
	}

	size_t commentStartIndex = std::string::npos;

	for(size_t i = 0; i < data.length();i++) {
		if(data[i] == ' ' || data[i] == '\t') {
			continue;
		}

		if(data[i] == comment[0]) {
			commentStartIndex = i;
			break;
		}
		else {
			return false;
		}
	}

	if(commentStartIndex == std::string::npos || data.length() - commentStartIndex < comment.length()) {
		return false;
	}

	for(size_t i = commentStartIndex; i < data.length(); i++) {
		if(i - commentStartIndex >= comment.length()) {
			break;
		}

		if(data[i] != comment[i - commentStartIndex]) {
			return false;
		}
	}

	return true;
}

bool Utilities::isURL(const std::string & data) {
	static const std::regex URL_ADDRESS_REGEX("https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{1,256}\\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()!@:%_\\+.~#?&\\/\\/=]*)");

	if(data.empty()) {
		return false;
	}

	return std::regex_match(data, URL_ADDRESS_REGEX);
}

bool Utilities::isEmailAddress(const std::string & data) {
	static const std::regex EMAIL_ADDRESS_REGEX("(([^<>()\\[\\]\\.,;:\\s@\"]+(\\.[^<>()\\[\\]\\.,;:\\s@\"]+)*)|(\".+\"))@((\\[[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}])|(([a-zA-Z\\-0-9]+\\.)+[a-zA-Z]{2,}))");

	if(data.empty()) {
		return false;
	}

	return std::regex_match(data, EMAIL_ADDRESS_REGEX);
}

bool Utilities::isIPV4Address(const std::string & data) {
	static const std::regex IPV4_ADDRESS_REGEX("(\\b25[0-5]|\\b2[0-4][0-9]|\\b[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}");

	if(data.empty()) {
		return false;
	}

	return std::regex_match(data, IPV4_ADDRESS_REGEX);
}

bool Utilities::isIPV6Address(const std::string & data) {
	static const std::regex IPV6_ADDRESS_REGEX("(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))");

	if(data.empty()) {
		return false;
	}

	return std::regex_match(data, IPV6_ADDRESS_REGEX);
}

bool Utilities::isMACAddress(const std::string & data) {
	static const std::regex MAC_ADDRESS_REGEX("^[a-fA-F0-9]{2}(:[a-fA-F0-9]{2}){5}$");

	if(data.empty()) {
		return false;
	}

	return std::regex_match(data, MAC_ADDRESS_REGEX);
}

std::string Utilities::getVariableID(std::string_view data) {
	if(data.empty()) {
		return std::string();
	}

	std::string formattedData(Utilities::trimString(data));

	if(formattedData.empty()) {
		return std::string();
	}

	size_t separatorIndex = formattedData.find_first_of(":=");

	return separatorIndex == std::string::npos ? std::string() : Utilities::trimString(formattedData.substr(0, separatorIndex));
}

std::string Utilities::getVariableValue(std::string_view data) {
	if(data.empty()) {
		return std::string();
	}

	std::string formattedData(Utilities::trimString(data));

	if(formattedData.empty()) {
		return std::string();
	}

	size_t separatorIndex = formattedData.find_first_of(":=");

	return separatorIndex == std::string::npos ? std::string() : Utilities::trimString(formattedData.substr(separatorIndex + 1, formattedData.length() - (separatorIndex + 1)));
}

bool Utilities::isValidIntegerNumber(const std::string & data) {
	static const std::regex INTEGER_NUMBER_REGEX("^((-?[1-9][0-9]*)|0)$");

	if(data.empty()) {
		return false;
	}

	return std::regex_match(data, INTEGER_NUMBER_REGEX);
}

bool Utilities::isValidWholeNumber(const std::string & data) {
	static const std::regex WHOLE_NUMBER_REGEX("^(([1-9][0-9]*)|0)$");

	if(data.empty()) {
		return false;
	}

	return std::regex_match(data, WHOLE_NUMBER_REGEX);
}

bool Utilities::isValidRealNumber(const std::string & data) {
	static const std::regex REAL_NUMBER_REGEX("^((([-]?[1-9][0-9]*)|0)([.][0-9]+)?)$");

	if(data.empty()) {
		return false;
	}

	return std::regex_match(data, REAL_NUMBER_REGEX);
}

bool Utilities::parseBoolean(std::string_view data, bool * error) {
	if(data.empty()) {
		if(error != nullptr) {
			*error = true;
		}

		return false;
	}

	std::string trimmedData(Utilities::trimString(data));

	if(trimmedData.empty()) {
		if(error != nullptr) {
			*error = true;
		}

		return false;
	}

	if(trimmedData.length() == 1) {
		if(trimmedData[0] == 't' ||
		   trimmedData[0] == '1' ||
		   trimmedData[0] == 'y') {
			return true;
		}
		else if(trimmedData[0] == 'f' ||
				trimmedData[0] == '0' ||
				trimmedData[0] == 'n') {
			return false;
		}
	}
	else {
		if(Utilities::areStringsEqualIgnoreCase(trimmedData, "true") ||
		   Utilities::areStringsEqualIgnoreCase(trimmedData, "on") ||
		   Utilities::areStringsEqualIgnoreCase(trimmedData, "yes")) {
			return true;
		}
		else if(Utilities::areStringsEqualIgnoreCase(trimmedData, "false") ||
				Utilities::areStringsEqualIgnoreCase(trimmedData, "off") ||
				Utilities::areStringsEqualIgnoreCase(trimmedData, "no")) {
			return false;
		}
	}

	if(error != nullptr) {
		*error = true;
	}

	return false;
}

std::optional<bool> Utilities::parseBoolean(std::string_view data) {
	bool error = false;

	bool value = Utilities::parseBoolean(data, &error);

	if(error) {
		return {};
	}

	return value;
}

int8_t Utilities::parseByte(const std::string & data, bool * error) {
	bool internalError = false;
	int32_t value = parseInteger(data, &internalError);

	if(internalError || value < std::numeric_limits<int8_t>::min() || value > std::numeric_limits<int8_t>::max()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return static_cast<int8_t>(value);
}

std::optional<int8_t> Utilities::parseByte(const std::string & data) {
	bool error = false;

	int8_t value = Utilities::parseByte(data, &error);

	if(error) {
		return {};
	}

	return value;
}

uint8_t Utilities::parseUnsignedByte(const std::string & data, bool * error) {
	bool internalError = false;
	uint32_t value = parseUnsignedInteger(data, &internalError);

	if(internalError || value > std::numeric_limits<uint8_t>::max()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return static_cast<uint8_t>(value);
}

std::optional<uint8_t> Utilities::parseUnsignedByte(const std::string & data) {
	bool error = false;

	uint8_t value = Utilities::parseUnsignedByte(data, &error);

	if(error) {
		return {};
	}

	return value;
}

int16_t Utilities::parseShort(const std::string & data, bool * error) {
	bool internalError = false;
	int32_t value = parseInteger(data, &internalError);

	if(internalError || value < std::numeric_limits<int16_t>::min() || value > std::numeric_limits<int16_t>::max()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return static_cast<int16_t>(value);
}

std::optional<int16_t> Utilities::parseShort(const std::string & data) {
	bool error = false;

	int16_t value = Utilities::parseShort(data, &error);

	if(error) {
		return {};
	}

	return value;
}

uint16_t Utilities::parseUnsignedShort(const std::string & data, bool * error) {
	bool internalError = false;
	uint32_t value = parseUnsignedInteger(data, &internalError);

	if(internalError || value > std::numeric_limits<uint16_t>::max()) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return static_cast<uint16_t>(value);
}

std::optional<uint16_t> Utilities::parseUnsignedShort(const std::string & data) {
	bool error = false;

	uint16_t value = Utilities::parseUnsignedShort(data, &error);

	if(error) {
		return {};
	}

	return value;
}

int32_t Utilities::parseInteger(const std::string & data, bool * error) {
	if(!Utilities::isValidIntegerNumber(data)) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return std::stol(data);
}

std::optional<int32_t> Utilities::parseInteger(const std::string & data) {
	bool error = false;

	int32_t value = Utilities::parseInteger(data, &error);

	if(error) {
		return {};
	}

	return value;
}

uint32_t Utilities::parseUnsignedInteger(const std::string & data, bool * error) {
	if(!Utilities::isValidWholeNumber(data)) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return std::stoul(data);
}

std::optional<uint32_t> Utilities::parseUnsignedInteger(const std::string & data) {
	bool error = false;

	uint32_t value = Utilities::parseUnsignedInteger(data, &error);

	if(error) {
		return {};
	}

	return value;
}

int64_t Utilities::parseLong(const std::string & data, bool * error) {
	if(!Utilities::isValidIntegerNumber(data)) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return std::stoll(data);
}

std::optional<int64_t> Utilities::parseLong(const std::string & data) {
	bool error = false;

	int64_t value = Utilities::parseLong(data, &error);

	if(error) {
		return {};
	}

	return value;
}

uint64_t Utilities::parseUnsignedLong(const std::string & data, bool * error) {
	if(!Utilities::isValidIntegerNumber(data)) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return std::stoull(data);
}

std::optional<uint64_t> Utilities::parseUnsignedLong(const std::string & data) {
	bool error = false;

	uint64_t value = Utilities::parseUnsignedLong(data, &error);

	if(error) {
		return {};
	}

	return value;
}

float Utilities::parseFloat(const std::string & data, bool * error) {
	if(!Utilities::isValidRealNumber(data)) {
		if(error != nullptr) {
			*error = true;
		}

		return 0.0f;
	}

	return std::stof(data);
}

std::optional<float> Utilities::parseFloat(const std::string & data) {
	bool error = false;

	float value = Utilities::parseFloat(data, &error);

	if(error) {
		return {};
	}

	return value;
}

double Utilities::parseDouble(const std::string & data, bool * error) {
	if(!Utilities::isValidRealNumber(data)) {
		if(error != nullptr) {
			*error = true;
		}

		return 0.0;
	}

	return std::stod(data);
}

std::optional<double> Utilities::parseDouble(const std::string & data) {
	bool error = false;

	double value = Utilities::parseDouble(data, &error);

	if(error) {
		return {};
	}

	return value;
}

std::optional<std::vector<std::string>> Utilities::parseVersion(const std::string & value, bool trimTrailingZeroes) {
	static const std::regex versionSplitRegex("[. \t]+");

	if(value.empty()) {
		return {};
	}

	std::vector<std::string> versionParts(Utilities::regularExpressionStringSplit(Utilities::trimString(value), versionSplitRegex));

	if(versionParts.empty()) {
		return {};
	}

	std::vector<std::string> version;

	for(size_t i = 0; i < versionParts.size(); i++) {
		if(Utilities::isValidWholeNumber(versionParts[i])) {
			std::optional<uint32_t> optionalNumberPart(Utilities::parseUnsignedInteger(versionParts[i]));

			if(!optionalNumberPart.has_value()) {
				continue;
			}

			version.push_back(std::to_string(*optionalNumberPart));
		}
		else {
			version.push_back(versionParts[i]);
		}
	}

	if(trimTrailingZeroes) {
		while(true) {
			if(version.size() <= 1) {
				break;
			}

			if(version[version.size() - 1] == "0") {
				version.pop_back();
			}
			else {
				break;
			}
		}
	}

	if(version.empty()) {
		return {};
	}

	return version;
}

std::optional<int8_t> Utilities::compareVersions(const std::string & v1, const std::string & v2, bool caseSensitive) {
	std::optional<std::vector<std::string>> optionalVersion1(Utilities::parseVersion(v1));

	if(!optionalVersion1.has_value()) {
		return {};
	}

	std::optional<std::vector<std::string>> optionalVersion2(Utilities::parseVersion(v2));

	if(!optionalVersion2.has_value()) {
		return {};
	}

	std::vector<std::string> version1(*optionalVersion1);
	std::vector<std::string> version2(*optionalVersion2);

	size_t index = 0;

	while(true) {
		if(index >= version1.size()) {
			if(version1.size() == version2.size()) {
				return 0;
			}

			for(size_t i = index; i < version2.size(); i++) {
				if(version2[i] != "0") {
					return -1;
				}
			}

			return 0;
		}

		if(index >= version2.size()) {
			for(size_t i = index; i < version1.size(); i++) {
				if(version1[i] != "0") {
					return 1;
				}
			}

			return 0;
		}

		std::optional<uint32_t> optionalNumberPartA(Utilities::parseInteger(version1[index]));
		std::optional<uint32_t> optionalNumberPartB(Utilities::parseInteger(version2[index]));

		if(optionalNumberPartA.has_value()) {
			if(!optionalNumberPartB.has_value()) {
				return -1;
			}
		}
		else {
			if(optionalNumberPartB.has_value()) {
				return 1;
			}
		}

		std::string formattedA(caseSensitive ? version1[index] : Utilities::toUpperCase(version1[index]));
		std::string formattedB(caseSensitive ? version2[index] : Utilities::toUpperCase(version2[index]));

		if(formattedA > formattedB) {
			return 1;
		}
		else if(formattedA < formattedB) {
			return -1;
		}

		index++;
	}
}

std::string_view Utilities::readLine(std::string_view data, size_t & offset) {
	if(data.empty() || offset >= data.length()) {
		return {};
	}

	bool hasCarriageReturn = false;
	size_t newLineIndex = data.find_first_of("\n", offset);

	if(newLineIndex == std::string::npos) {
		std::string_view line(data.data() + offset, data.length() - offset);
		offset = data.length();

		return line;
	}
	else if(newLineIndex != 0 && data[newLineIndex - 1] == '\r') {
		hasCarriageReturn = true;
		newLineIndex--;
	}

	std::string_view line(data.data() + offset, newLineIndex - offset);
	offset = newLineIndex + (hasCarriageReturn ? 2 : 1);

	return line;
}

#include "StringUtilities.h"

#include <codecvt>
#include <locale>
#include <sstream>

unsigned int Utilities::stringLength(const char * s) {
	return s == nullptr ? 0 : strlen(s);
}

const char * Utilities::toString(int value) {
	static char buffer[12];
#if _WIN32
	sprintf_s(buffer, 12, "%d", value);
#else
	sprintf(buffer, "%d", value);
#endif // _WIN32

	return buffer;
}

const char * Utilities::toString(double value) {
	static const int MAX_DOUBLE_LENGTH = DBL_MANT_DIG - DBL_MIN_EXP + 4;

	static char buffer[MAX_DOUBLE_LENGTH];
#if _WIN32
	sprintf_s(buffer, MAX_DOUBLE_LENGTH, "%f", value);
#else
	sprintf(buffer, "%f", value);
#endif // _WIN32

	return buffer;
}

bool Utilities::copyString(char * destination, int size, const char * source) {
	if(source == nullptr || destination == nullptr || size < 1) { return false; }

#if _WIN32
	return strcpy_s(destination, size, source) == 0;
#else
	return strcpy(destination, source) == 0;
#endif // _WIN32
}

char * Utilities::copyString(const char * data) {
	if(data == nullptr) { return nullptr; }

	int bufferSize = Utilities::stringLength(data) + 1;
	char * newData = new char[bufferSize];
	if(Utilities::copyString(newData, bufferSize, data)) {
		return newData;
	}
	return nullptr;
}

// trims whitespace off of the front and end of string passed into it, and returns a copy of the trimmed string
char * Utilities::trimCopyString(const char * data) {
	// verify the string
	if(data == nullptr) { return nullptr; }
	char * newData;
	int length = Utilities::stringLength(data);
	if(length == 0) {
		newData = new char[1];
		*newData = '\0';
		return newData;
	}

	// find the new start and end of the string and verify that they do not overlap (0 length string)
	const char * head = data;
	const char * tail = data + (sizeof(char) * length) - 1;
	int startPos = 0, endPos = length - 1;
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
	newData = new char[endPos - startPos + 2];
	char * temp = newData;
	for(int i=startPos;i<=endPos;i++) {
		*temp = data[i];
		temp += sizeof(char);
	}
	*temp = '\0';
	return newData;
}

std::string Utilities::trimString(const std::string & data, bool trimWhiteSpace, bool trimNewLines) {
	if(data.empty()) { return std::string(); }
	if(trimWhiteSpace == false && trimNewLines == false) { return data; }
	if(data.length() == 1 && ((trimWhiteSpace && (data[0] == ' ' || data[0] == '\t')) || (trimNewLines && (data[0] == '\n' || data[0] == '\r')))) { return std::string(); }
	
	int start = 0;
	int end = 0;
	
	for(int i=0;i<static_cast<int>(data.length());i++) {
		start = i;
		
		if(!((trimWhiteSpace && (data[i] == ' ' || data[i] == '\t')) || (trimNewLines && (data[i] == '\n' || data[i] == '\r')))) {
			break;
		}
	}
	
	for(int i=static_cast<int>(data.length())-1;i>=0;i--) {
		end = i;
		
		if(!((trimWhiteSpace && (data[i] == ' ' || data[i] == '\t')) || (trimNewLines && (data[i] == '\n' || data[i] == '\r')))) {
			break;
		}
	}
	
	if(start > end) { return std::string(); }
	
	return data.substr(start, end - start + 1);
}

char * Utilities::substring(const char * data, int start, int end) {
	if(data == nullptr || start > end) { return nullptr; }

	int dataLength = Utilities::stringLength(data);

	if(dataLength == 0) {
		char * newString = new char[1];
		newString[0] = '\0';

		return newString;
	}

	int startPos = start < 0 ? 0 : start;
	int endPos = end > dataLength ? dataLength : end;

	char * newString = new char[endPos - startPos + 1];
	int x = 0;
	for(int i=startPos;i<endPos;i++) {
		newString[x++] = data[i];
	}
	newString[x++] = '\0';

	return newString;
}

std::string Utilities::substring(const std::string & data, int start, int end) {
	if(data.empty() || start > end) { return std::string(); }

	int startPos = start < 0 ? 0 : start;
	int endPos = end > static_cast<int>(data.length()) ? static_cast<int>(data.length()) : end;

	return data.substr(startPos, endPos - startPos);
}

std::string Utilities::wideStringToString(const std::wstring & wideString) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(wideString);
}

std::wstring Utilities::stringToWideString(const std::string & string) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(string);
}

int Utilities::compareStrings(const std::string & s1, const std::string & s2, bool caseSensitive) {
	if(caseSensitive) {
		return strcmp(s1.c_str(), s2.c_str());
	}

#if _WIN32
	return _stricmp(s1.c_str(), s2.c_str());
#else
	return strcasecmp(s1, s2);
#endif // _WIN32
}

int Utilities::compareStringsIgnoreCase(const std::string & s1, const std::string & s2) {
	return Utilities::compareStrings(s1, s2, false);
}

bool Utilities::isComment(const std::string & data, const std::string & comment) {
	if(comment.empty()) { return false; }

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

std::string Utilities::getVariableID(const std::string & data) {
	if(data.empty()) { return std::string(); }

	std::string formattedData = Utilities::trimString(data);

	if(formattedData.empty()) { return std::string(); }

	int separatorIndex = -1;
	for(unsigned int i=0;i<formattedData.length();i++) {
		if(formattedData[i] == ':' || formattedData[i] == '=') {
			separatorIndex = i;
			break;
		}
	}
	if(separatorIndex < 0) { return std::string(); }

	return Utilities::trimString(Utilities::substring(formattedData, 0, separatorIndex));
}

std::string Utilities::getVariableValue(const std::string & data) {
	if(data.empty()) { return std::string(); }

	std::string formattedData = Utilities::trimString(data);

	if(formattedData.empty()) { return std::string(); }

	int separatorIndex = -1;
	for(unsigned int i=0;i<formattedData.length();i++) {
		if(formattedData[i] == ':' || formattedData[i] == '=') {
			separatorIndex = i;
			break;
		}
	}
	if(separatorIndex < 0) { return std::string(); }

	return Utilities::trimString(Utilities::substring(formattedData, separatorIndex + 1, formattedData.length()));
}

bool Utilities::parseBoolean(const std::string & data, bool * valid) {
	if(data.empty()) {
		if(valid != nullptr) { *valid = false; }
		return false;
	}

	char * trimmedData = Utilities::trimCopyString(data.c_str());
	if(Utilities::stringLength(trimmedData) == 0) {
		if(valid != nullptr) { *valid = false; }
		delete [] trimmedData;
		return false;
	}

	if(Utilities::stringLength(trimmedData) == 1) {
		if(trimmedData[0] == 't' ||
		   trimmedData[0] == '1' ||
		   trimmedData[0] == 'y') {
			if(valid != nullptr) { *valid = true; }
			delete [] trimmedData;
			return true;
		}
		else if(trimmedData[0] == 'f' ||
				trimmedData[0] == '0' ||
				trimmedData[0] == 'n') {
			if(valid != nullptr) { *valid = true; }
			delete [] trimmedData;
			return false;
		}
	}
	else {
		if(Utilities::compareStringsIgnoreCase(trimmedData, "true") == 0 ||
		   Utilities::compareStringsIgnoreCase(trimmedData, "on") == 0 ||
		   Utilities::compareStringsIgnoreCase(trimmedData, "yes") == 0) {
			if(valid != nullptr) { *valid = true; }
			delete [] trimmedData;
			return true;
		}
		else if(Utilities::compareStringsIgnoreCase(trimmedData, "false") == 0 ||
				Utilities::compareStringsIgnoreCase(trimmedData, "off") == 0 ||
				Utilities::compareStringsIgnoreCase(trimmedData, "no") == 0) {
			if(valid != nullptr) { *valid = true; }
			delete [] trimmedData;
			return false;
		}
	}

	if(valid != nullptr) { *valid = false; }
	delete [] trimmedData;
	return false;
}

int Utilities::parseInteger(const std::string & data, bool * valid) {
	if(data.empty()) {
		if(valid != nullptr) { *valid = false; }
		return 0;
	}

	char * trimmedData = Utilities::trimCopyString(data.c_str());
	if(Utilities::stringLength(trimmedData) == 0) {
		if(valid != nullptr) { *valid = false; }
		delete [] trimmedData;
		return 0;
	}

	for(size_t i = 0; i < Utilities::stringLength(trimmedData); i++) {
		if(!(trimmedData[i] == '-' || (trimmedData[i] >= '0' && trimmedData[i] <= '9'))) {
			if(valid != nullptr) { *valid = false; }
			delete [] trimmedData;
			return 0;
		}
	}

	if(valid != nullptr) { *valid = true; }
	int value = atoi(trimmedData);

	delete [] trimmedData;

	return value;
}

float Utilities::parseFloat(const std::string & data, bool * valid) {
	if(data.empty()) {
		if(valid != nullptr) { *valid = false; }
		return 0.0f;
	}

	char * trimmedData = Utilities::trimCopyString(data.c_str());
	if(Utilities::stringLength(trimmedData) == 0) {
		if(valid != nullptr) { *valid = false; }
		delete [] trimmedData;
		return 0.0f;
	}

	for(unsigned int i=0;i<Utilities::stringLength(trimmedData);i++) {
		if(!(trimmedData[i] == '-' || trimmedData[i] == '.' || (trimmedData[i] >= '0' && trimmedData[i] <= '9'))) {
			if(valid != nullptr) { *valid = false; }
			delete [] trimmedData;
			return 0.0f;
		}
	}

	if(valid != nullptr) { *valid = true; }
	float value = static_cast<float>(atof(trimmedData));

	delete [] trimmedData;

	return value;
}

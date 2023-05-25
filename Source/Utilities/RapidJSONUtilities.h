#ifndef _RAPID_JSON_UTILITIES_H_
#define _RAPID_JSON_UTILITIES_H_

#include <rapidjson/document.h>

#include <any>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Utilities {

	std::string getParseErrorName(rapidjson::ParseErrorCode parseErrorCode);
	std::string_view typeToString(rapidjson::Type type);
	std::string valueToString(const rapidjson::Value & value, bool pretty = true);
	std::optional<rapidjson::Value> anyToJSONValue(const std::any & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull = true);
	rapidjson::Document anyVectorToJSONDocument(const std::vector<std::any> & values, bool allowNull = true);
	rapidjson::Value anyVectorToJSONValue(const std::vector<std::any> & values, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull = true);
	rapidjson::Document anyMapToJSONDocument(const std::map<std::string, std::any> & valueMap, bool allowNull = true);
	rapidjson::Value anyMapToJSONValue(const std::map<std::string, std::any> & valueMap, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull = true);
	std::any jsonValueToAny(const rapidjson::Value & value);
	std::vector<std::any> jsonArrayToAnyVector(const rapidjson::Value & value);
	std::map<std::string, std::any> jsonObjectToAnyMap(const rapidjson::Value & value);
	std::optional<rapidjson::Document> loadJSONDocumentFrom(const std::string & filePath);
	bool saveJSONValueTo(const rapidjson::Value & jsonValue, const std::string & filePath, bool overwrite = true, bool createParentDirectories = true);

}

#endif // _RAPID_JSON_UTILITIES_H_

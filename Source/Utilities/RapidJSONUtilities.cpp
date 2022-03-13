#include "RapidJSONUtilities.h"

#include <fmt/core.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

std::string_view Utilities::typeToString(rapidjson::Type type) {
	switch(type) {
		case rapidjson::kNullType:
			return "null";

		case rapidjson::kFalseType:
		case rapidjson::kTrueType:
			return "boolean";

		case rapidjson::kObjectType:
			return "object";

		case rapidjson::kArrayType:
			return "array";

		case rapidjson::kStringType:
			return "string";

		case rapidjson::kNumberType:
			return "number";
	}

	return std::string_view();
}

std::string Utilities::valueToString(const rapidjson::Value & value) {
	switch(value.GetType()) {
		case rapidjson::kNullType:
			return "null";

		case rapidjson::kFalseType:
		case rapidjson::kTrueType:
			return value.GetBool() ? "true" : "false";

		case rapidjson::kObjectType:
		case rapidjson::kArrayType: {
			rapidjson::StringBuffer stringBuffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
			value.Accept(writer);
			return stringBuffer.GetString();
		}

		case rapidjson::kStringType:
			return fmt::format("\"{}\"", value.GetString());

		case rapidjson::kNumberType: {
			if(value.IsDouble()) {
				return fmt::format("{}", value.GetDouble());
			}
			else if(value.IsInt()) {
				return fmt::format("{}", value.GetInt());
			}
			else if(value.IsUint()) {
				return fmt::format("{}", value.GetUint());
			}
			else if(value.IsInt64()) {
				return fmt::format("{}", value.GetInt64());
			}
			else if(value.IsUint64()) {
				return fmt::format("{}", value.GetUint64());
			}
		}
	}

	return std::string();
}

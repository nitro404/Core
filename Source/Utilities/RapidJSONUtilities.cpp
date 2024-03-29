#include "RapidJSONUtilities.h"

#include "FileUtilities.h"

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <functional>
#include <fstream>
#include <typeindex>

using AnyToJSONConverterFunction = std::function<void (const std::any & any, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull)>;

template<class T, class F>
static inline std::pair<const std::type_index, AnyToJSONConverterFunction> toAnyToJSONConverter(const F & f) {
	return {
		std::type_index(typeid(T)),
		[&f](const std::any & any, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
			if constexpr(std::is_void_v<T>) {
				f(outputValue, allowNull);
			}
			else {
				f(std::any_cast<const T &>(any), outputValue, allocator, allowNull);
			}
		}
	};
}

static const std::unordered_map<std::type_index, AnyToJSONConverterFunction> anyToJSONConverters {
	toAnyToJSONConverter<void> ([](std::optional<rapidjson::Value> & outputValue, bool allowNull) {
		if(allowNull) {
			outputValue = rapidjson::Value(rapidjson::kNullType);
		}
		else {
			outputValue.reset();
		}
	}),
	toAnyToJSONConverter<bool> ([](bool value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<int8_t> ([](int8_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<uint8_t> ([](uint8_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<int16_t> ([](int16_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<uint16_t> ([](uint16_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<int32_t> ([](int32_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<uint32_t> ([](uint32_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<int64_t> ([](int64_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<uint64_t> ([](uint64_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<size_t> ([](size_t value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<float> ([](float value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<double> ([](double value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value);
	}),
	toAnyToJSONConverter<const char *> ([](const char * value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		if(value == nullptr) {
			if(allowNull) {
				outputValue = rapidjson::Value(rapidjson::kNullType);
			}
			else {
				outputValue.reset();
			}
		}
		else {
			outputValue = rapidjson::Value(value, allocator);
		}
	}),
	toAnyToJSONConverter<std::string_view> ([](std::string_view value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		std::string stringValue(value);
		outputValue = rapidjson::Value(stringValue.c_str(), allocator);
	}),
	toAnyToJSONConverter<std::string> ([](std::string value, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(value.c_str(), allocator);
	}),
	toAnyToJSONConverter<std::vector<std::any>> ([](std::vector<std::any> values, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(rapidjson::kArrayType);

		for(std::vector<std::any>::const_iterator i = values.begin(); i != values.end(); ++i) {
			const auto anyToJSONConverter = anyToJSONConverters.find(std::type_index(i->type()));

			if(anyToJSONConverter == anyToJSONConverters.cend()) {
				spdlog::warn("Failed to convert any to JSON, no converter registered for type: '{}'.", i->type().name());
				continue;
			}

			std::optional<rapidjson::Value> arrayEntryValue;

			anyToJSONConverter->second(*i, arrayEntryValue, allocator, allowNull);

			if(arrayEntryValue.has_value()) {
				outputValue->PushBack(arrayEntryValue.value(), allocator);
			}
		}
	}),
	toAnyToJSONConverter<std::map<std::string, std::any>> ([](std::map<std::string, std::any> valueMap, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
		outputValue = rapidjson::Value(rapidjson::kObjectType);

		for(std::map<std::string, std::any>::const_iterator i = valueMap.begin(); i != valueMap.end(); ++i) {
			const auto anyToJSONConverter = anyToJSONConverters.find(std::type_index(i->second.type()));

			if(anyToJSONConverter == anyToJSONConverters.cend()) {
				spdlog::warn("Failed to convert any to JSON, no converter registered for type: '{}'.", i->second.type().name());
				continue;
			}

			std::optional<rapidjson::Value> mapEntryValue;

			anyToJSONConverter->second(i->second, mapEntryValue, allocator, allowNull);

			if(mapEntryValue.has_value()) {
				rapidjson::Value mapEntryName(i->first.c_str(), allocator);
				outputValue->AddMember(mapEntryName, mapEntryValue.value(), allocator);
			}
		}
	})
};

std::string Utilities::getParseErrorName(rapidjson::ParseErrorCode parseErrorCode) {
	switch(parseErrorCode) {
		case rapidjson::kParseErrorNone:
			return "None";
		case rapidjson::kParseErrorDocumentEmpty:
			return "Document Empty";
		case rapidjson::kParseErrorDocumentRootNotSingular:
			return "Document Root Not Singular";
		case rapidjson::kParseErrorValueInvalid:
			return "Value Invalid";
		case rapidjson::kParseErrorObjectMissName:
			return "Object Missing Name";
		case rapidjson::kParseErrorObjectMissColon:
			return "Object Missing Colon";
		case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket:
			return "Object Missing Comma or Curly Bracket";
		case rapidjson::kParseErrorArrayMissCommaOrSquareBracket:
			return "Array Missing Comma or Square Bracket";
		case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex:
			return "String Unicode Escape Invalid Hexadecimal";
		case rapidjson::kParseErrorStringUnicodeSurrogateInvalid:
			return "String Unicode Surrogate Invalid";
		case rapidjson::kParseErrorStringEscapeInvalid:
			return "String Escape Invalid";
		case rapidjson::kParseErrorStringMissQuotationMark:
			return "String Missing Quotation Mark";
		case rapidjson::kParseErrorStringInvalidEncoding:
			return "String Invalid Encoding";
		case rapidjson::kParseErrorNumberTooBig:
			return "Number Too Big";
		case rapidjson::kParseErrorNumberMissFraction:
			return "Number Missing Fraction";
		case rapidjson::kParseErrorNumberMissExponent:
			return "Number Missing Exponent";
		case rapidjson::kParseErrorTermination:
			return "Termination";
		case rapidjson::kParseErrorUnspecificSyntaxError:
			return "Unspecific Syntax Error";
	}

	return {};
}

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

std::string Utilities::valueToString(const rapidjson::Value & value, bool pretty) {
	rapidjson::StringBuffer stringBuffer;

	if(pretty) {
		rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyStringBufferWriter(stringBuffer);
		prettyStringBufferWriter.SetIndent('\t', 1);
		value.Accept(prettyStringBufferWriter);
	}
	else {
		rapidjson::Writer<rapidjson::StringBuffer> stringBufferWriter(stringBuffer);
		value.Accept(stringBufferWriter);
	}

	return stringBuffer.GetString();
}

std::optional<rapidjson::Value> Utilities::anyToJSONValue(const std::any & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
	const auto anyToJSONConverter = anyToJSONConverters.find(std::type_index(value.type()));

	if(anyToJSONConverter == anyToJSONConverters.cend()) {
		spdlog::warn("Failed to convert any to JSON value, no converter registered for type: '{}'.", value.type().name());
		return {};
	}

	std::optional<rapidjson::Value> optionalValue;

	anyToJSONConverter->second(value, optionalValue, allocator, allowNull);

	if(!optionalValue.has_value()) {
		return {};
	}

	return std::move(optionalValue.value());
}

rapidjson::Document Utilities::anyVectorToJSONDocument(const std::vector<std::any> & values, bool allowNull) {
	rapidjson::Document vectorDocument(rapidjson::kArrayType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = vectorDocument.GetAllocator();

	for(std::vector<std::any>::const_iterator i = values.begin(); i != values.end(); ++i) {
		const auto anyToJSONConverter = anyToJSONConverters.find(std::type_index(i->type()));

		if(anyToJSONConverter == anyToJSONConverters.cend()) {
			spdlog::warn("Failed to convert any to JSON, no converter registered for type: '{}'.", i->type().name());
			continue;
		}

		std::optional<rapidjson::Value> arrayEntryValue;

		anyToJSONConverter->second(*i, arrayEntryValue, allocator, allowNull);

		if(arrayEntryValue.has_value()) {
			vectorDocument.PushBack(arrayEntryValue.value(), allocator);
		}
	}

	return std::move(vectorDocument);
}

rapidjson::Value Utilities::anyVectorToJSONValue(const std::vector<std::any> & values, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
	const auto anyVectorToJSONConverter = anyToJSONConverters.find(std::type_index(typeid(std::vector<std::any>)));

	if(anyVectorToJSONConverter == anyToJSONConverters.cend()) {
		spdlog::warn("Failed to convert any vector to JSON value, no converter registered for type: '{}'.", typeid(std::vector<std::any>).name());
		return {};
	}

	std::optional<rapidjson::Value> optionalVectorValue;

	anyVectorToJSONConverter->second(values, optionalVectorValue, allocator, allowNull);

	if(!optionalVectorValue.has_value()) {
		return {};
	}

	return std::move(optionalVectorValue.value());
}

rapidjson::Document Utilities::anyMapToJSONDocument(const std::map<std::string, std::any> & valueMap, bool allowNull) {
	rapidjson::Document mapDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = mapDocument.GetAllocator();

	for(std::map<std::string, std::any>::const_iterator i = valueMap.begin(); i != valueMap.end(); ++i) {
		const auto anyToJSONConverter = anyToJSONConverters.find(std::type_index(i->second.type()));

		if(anyToJSONConverter == anyToJSONConverters.cend()) {
			spdlog::warn("Failed to convert any to JSON, no converter registered for type: '{}'.", i->second.type().name());
			continue;
		}

		std::optional<rapidjson::Value> mapEntryValue;

		anyToJSONConverter->second(i->second, mapEntryValue, allocator, allowNull);

		if(mapEntryValue.has_value()) {
			rapidjson::Value mapEntryName(i->first.c_str(), allocator);
			mapDocument.AddMember(mapEntryName, mapEntryValue.value(), allocator);
		}
	}

	return std::move(mapDocument);
}

rapidjson::Value Utilities::anyMapToJSONValue(const std::map<std::string, std::any> & valueMap, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
	const auto anyMapToJSONConverter = anyToJSONConverters.find(std::type_index(typeid(std::map<std::string, std::any>)));

	if(anyMapToJSONConverter == anyToJSONConverters.cend()) {
		spdlog::warn("Failed to convert any map to JSON value, no converter registered for type: '{}'.", typeid(std::map<std::string, std::any>).name());
		return {};
	}

	std::optional<rapidjson::Value> optionalMapValue;

	anyMapToJSONConverter->second(valueMap, optionalMapValue, allocator, allowNull);

	if(!optionalMapValue.has_value()) {
		return {};
	}

	return std::move(optionalMapValue.value());
}

std::any Utilities::jsonValueToAny(const rapidjson::Value & value) {
	switch(value.GetType()) {
		case rapidjson::kNullType: {
			return {};
		}

		case rapidjson::kFalseType:
		case rapidjson::kTrueType: {
			return value.GetBool();
		}

		case rapidjson::kObjectType: {
			return jsonObjectToAnyMap(value);
		}

		case rapidjson::kArrayType: {
			return jsonArrayToAnyVector(value);
		}

		case rapidjson::kStringType: {
			return std::string(value.GetString());
		}

		case rapidjson::kNumberType: {
			if(value.IsDouble()) {
				return value.GetDouble();
			}
			else if(value.IsInt()) {
				return static_cast<int32_t>(value.GetInt());
			}
			else if(value.IsUint()) {
				return static_cast<uint32_t>(value.GetUint());
			}
			else if(value.IsInt64()) {
				return value.GetInt64();
			}
			else if(value.IsUint64()) {
				return value.GetUint64();
			}
		}
	}

	return {};
}

std::vector<std::any> Utilities::jsonArrayToAnyVector(const rapidjson::Value & value) {
	if(!value.IsArray()) {
		return {};
	}

	std::vector<std::any> anyVector;

	for(rapidjson::Value::ConstValueIterator i = value.Begin(); i != value.End(); ++i) {
		anyVector.emplace_back(jsonValueToAny(*i));
	}

	return anyVector;
}

std::map<std::string, std::any> Utilities::jsonObjectToAnyMap(const rapidjson::Value & value) {
	if(!value.IsObject()) {
		return {};
	}

	std::map<std::string, std::any> anyMap;

	for(rapidjson::Value::ConstMemberIterator i = value.MemberBegin(); i != value.MemberEnd(); ++i) {
		anyMap[i->name.GetString()] = jsonValueToAny(i->value);
	}

	return anyMap;
}

std::optional<rapidjson::Document> Utilities::loadJSONDocumentFrom(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return {};
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return {};
	}

	rapidjson::Document jsonDocument;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);

	if(jsonDocument.ParseStream(fileStreamWrapper).HasParseError()) {
		return {};
	}

	fileStream.close();

	return jsonDocument;
}

bool Utilities::saveJSONValueTo(const rapidjson::Value & jsonValue, const std::string & filePath, bool overwrite, bool createParentDirectories) {
	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	if(createParentDirectories) {
		std::error_code errorCode;
		Utilities::createDirectoryStructureForFilePath(filePath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create file destination directory structure for file path '{}': {}", filePath, errorCode.message());
			return false;
		}
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	jsonValue.Accept(fileStreamWriter);

	fileStream.close();

	return true;
}

#include "RapidJSONUtilities.h"

#include <fmt/core.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <functional>
#include <typeindex>

using AnyToJSONConverterFunction = std::function<void (const std::any & any, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull)>;

template<class T, class F>
static inline std::pair<const std::type_index, AnyToJSONConverterFunction> toAnyToJSONConverter(const F & f) {
	return {
		std::type_index(typeid(T)),
		[g = f](const std::any & any, std::optional<rapidjson::Value> & outputValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator, bool allowNull) {
			if constexpr(std::is_void_v<T>) {
				g(outputValue, allowNull);
			}
			else {
				g(std::any_cast<const T &>(any), outputValue, allocator, allowNull);
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
				fmt::print("Failed to convert any to JSON, no converter registered for type: '{}'.\n", typeid(i->type()).name());
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
				fmt::print("Failed to convert any to JSON, no converter registered for type: '{}'.\n", typeid(i->second.type()).name());
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
	const auto anyToJSONConverter = anyToJSONConverters.find(std::type_index(typeid(value.type())));

	if(anyToJSONConverter == anyToJSONConverters.cend()) {
		fmt::print("Failed to convert any to JSON value, no converter registered for type: '{}'.\n", typeid(value.type()).name());
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
			fmt::print("Failed to convert any to JSON, no converter registered for type: '{}'.\n", typeid(i->type()).name());
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
		fmt::print("Failed to convert any vector to JSON value, no converter registered for type: '{}'.\n", typeid(std::vector<std::any>).name());
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
	rapidjson::Document mapDocument(rapidjson::kArrayType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = mapDocument.GetAllocator();

	for(std::map<std::string, std::any>::const_iterator i = valueMap.begin(); i != valueMap.end(); ++i) {
		const auto anyToJSONConverter = anyToJSONConverters.find(std::type_index(i->second.type()));

		if(anyToJSONConverter == anyToJSONConverters.cend()) {
			fmt::print("Failed to convert any to JSON, no converter registered for type: '{}'.\n", typeid(i->second.type()).name());
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
		fmt::print("Failed to convert any map to JSON value, no converter registered for type: '{}'.\n", typeid(std::map<std::string, std::any>).name());
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
			return value.GetString();
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

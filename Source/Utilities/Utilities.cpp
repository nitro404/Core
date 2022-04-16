#include "Utilities.h"

#include "StringUtilities.h"

#include <fmt/core.h>

#include <functional>
#include <map>
#include <regex>
#include <sstream>
#include <time.h>
#include <typeindex>
#include <vector>

static bool s_initialRandomize = true;

using AnyToStringConverterFunction = std::function<void (const std::any & any, std::string & outputString)>;

template<class T, class F>
static inline std::pair<const std::type_index, AnyToStringConverterFunction> toAnyToStringConverter(const F & f) {
	return {
		std::type_index(typeid(T)),
		[g = f](const std::any & any, std::string & outputString) {
			if constexpr(std::is_void_v<T>) {
				g(outputString);
			}
			else {
				g(std::any_cast<const T &>(any), outputString);
			}
		}
	};
}

static const std::unordered_map<std::type_index, AnyToStringConverterFunction> anyToStringConverters {
	toAnyToStringConverter<void>             ([](std::string & outputString)                         { outputString.clear(); }),
	toAnyToStringConverter<bool>             ([](bool value, std::string & outputString)             { outputString = value ? "true" : "false"; }),
	toAnyToStringConverter<int8_t>           ([](int8_t value, std::string & outputString)           { outputString = std::to_string(value); }),
	toAnyToStringConverter<uint8_t>          ([](uint8_t value, std::string & outputString)          { outputString = std::to_string(value); }),
	toAnyToStringConverter<int16_t>          ([](int16_t value, std::string & outputString)          { outputString = std::to_string(value); }),
	toAnyToStringConverter<uint16_t>         ([](uint16_t value, std::string & outputString)         { outputString = std::to_string(value); }),
	toAnyToStringConverter<int32_t>          ([](int32_t value, std::string & outputString)          { outputString = std::to_string(value); }),
	toAnyToStringConverter<uint32_t>         ([](uint32_t value, std::string & outputString)         { outputString = std::to_string(value); }),
	toAnyToStringConverter<int64_t>          ([](int64_t value, std::string & outputString)          { outputString = std::to_string(value); }),
	toAnyToStringConverter<uint64_t>         ([](uint64_t value, std::string & outputString)         { outputString = std::to_string(value); }),
	toAnyToStringConverter<size_t>           ([](size_t value, std::string & outputString)           { outputString = std::to_string(value); }),
	toAnyToStringConverter<float>            ([](float value, std::string & outputString)            { outputString = std::to_string(value); }),
	toAnyToStringConverter<double>           ([](double value, std::string & outputString)           { outputString = std::to_string(value); }),
	toAnyToStringConverter<const char *>     ([](const char * value, std::string & outputString)     { if(value == nullptr) { outputString.clear(); } else { outputString = value; } }),
	toAnyToStringConverter<std::string_view> ([](std::string_view value, std::string & outputString) { outputString = value; }),
	toAnyToStringConverter<std::string>      ([](std::string value, std::string & outputString)      { outputString = value; }),
	toAnyToStringConverter<std::vector<std::any>> ([](std::vector<std::any> values, std::string & outputString) {
		std::stringstream outputStringStream;

		outputStringStream << "[";

		bool firstEntry = true;

		for(std::vector<std::any>::const_iterator i = values.cbegin(); i != values.cend(); ++i) {
			const auto anyToStringConverter = anyToStringConverters.find(std::type_index(i->type()));

			if(anyToStringConverter == anyToStringConverters.cend()) {
				fmt::print("Failed to convert any to string, no converter registered for type: '{}'.\n", typeid(i->type()).name());
				continue;
			}

			if(firstEntry) {
				firstEntry = false;
			}
			else {
				outputStringStream << ", ";
			}

			std::string anyString;

			anyToStringConverter->second(*i, anyString);

			outputStringStream << anyString;
		}

		outputStringStream << "]";

		outputString = outputStringStream.str();
	}),
	toAnyToStringConverter<std::map<std::string, std::any>> ([](std::map<std::string, std::any> valueMap, std::string & outputString) {
		std::stringstream outputStringStream;

		outputStringStream << "{";

		bool firstEntry = true;

		for(std::map<std::string, std::any>::const_iterator i = valueMap.cbegin(); i != valueMap.cend(); ++i) {
			const auto anyToStringConverter = anyToStringConverters.find(std::type_index(i->second.type()));

			if(anyToStringConverter == anyToStringConverters.cend()) {
				fmt::print("Failed to convert any to string, no converter registered for type: '{}'.\n", typeid(i->second.type()).name());
				continue;
			}

			if(firstEntry) {
				firstEntry = false;
			}
			else {
				outputStringStream << ",";
			}

			std::string anyString;

			anyToStringConverter->second(i->second, anyString);

			outputStringStream << fmt::format(" {}: {}", i->first, anyString);
		}

		outputStringStream << " }";

		outputString = outputStringStream.str();
	})
};

void Utilities::randomizeSeed() {
	srand(static_cast<unsigned int>(time(NULL)));
}

void Utilities::randomSeed(unsigned int seed) {
	srand(seed);
}

int Utilities::randomInteger(int min, int max, bool randomize) {
	if(max <= min) { return min; }

	if(randomize || s_initialRandomize) {
		Utilities::randomizeSeed();
		s_initialRandomize = false;
	}

	return (rand() % (max - min + 1)) + min;
}

float Utilities::randomFloat(float min, float max, bool randomize) {
	if(max <= min) { return min; }

	if(randomize || s_initialRandomize) {
		Utilities::randomizeSeed();
		s_initialRandomize = false;
	}

	return ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min)) + min;
}

std::string Utilities::anyToString(const std::any & value) {
	const auto anyToStringConverter = anyToStringConverters.find(std::type_index(typeid(value.type())));

	if(anyToStringConverter == anyToStringConverters.cend()) {
		fmt::print("Failed to convert any to string value, no converter registered for type: '{}'.\n", typeid(value.type()).name());
		return {};
	}

	std::string anyString;

	anyToStringConverter->second(value, anyString);

	return anyString;
}

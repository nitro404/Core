#ifndef _RAPID_JSON_UTILITIES_H_
#define _RAPID_JSON_UTILITIES_H_

#include <rapidjson/document.h>

#include <string>
#include <string_view>

namespace Utilities {

	std::string_view typeToString(rapidjson::Type type);
	std::string valueToString(const rapidjson::Value & value);

}

#endif // _RAPID_JSON_UTILITIES_H_

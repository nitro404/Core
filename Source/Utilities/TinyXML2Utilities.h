#ifndef _TINYXML2_UTILITIES_H_
#define _TINYXML2_UTILITIES_H_

#include <tinyxml2.h>

#include <string>
#include <vector>

namespace Utilities {

	const tinyxml2::XMLElement * findXMLElementWithID(const tinyxml2::XMLElement * element, const std::string & id);
	const tinyxml2::XMLElement * findXMLElementWithName(const tinyxml2::XMLElement * element, const std::string & name);
	std::vector<const tinyxml2::XMLElement *> findXMLElementsWithName(const tinyxml2::XMLElement * element, const std::string & name);
	std::vector<const tinyxml2::XMLElement *> findXMLElementsWithAttributeValue(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue);

}

#endif // _TINYXML2_UTILITIES_H_

#ifndef _TINYXML2_UTILITIES_H_
#define _TINYXML2_UTILITIES_H_

#include <tinyxml2.h>

#include <string>
#include <vector>

namespace Utilities {

	bool doesXMLElementHaveID(const tinyxml2::XMLElement * element, const std::string & id);
	bool doesXMLElementHaveAttributeValue(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue);
	bool doesXMLElementHaveClassName(const tinyxml2::XMLElement * element, const std::string & className);
	bool doesXMLElementHaveClassNames(const tinyxml2::XMLElement * element, const std::vector<std::string> & classNames);
	bool doesXMLElementHaveFullText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive = false);
	bool doesXMLElementContainText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive = false);
	std::vector<std::string> getXMLElementClassNames(const tinyxml2::XMLElement * element);
	const tinyxml2::XMLElement * findXMLElementWithID(const tinyxml2::XMLElement * element, const std::string & id);
	const tinyxml2::XMLElement * findFirstXMLElementWithName(const tinyxml2::XMLElement * element, const std::string & name, bool caseSensitive = false);
	const tinyxml2::XMLElement * findFirstXMLElementWithAttributeValue(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue);
	const tinyxml2::XMLElement * findFirstXMLElementWithClassName(const tinyxml2::XMLElement * element, const std::string & className);
	const tinyxml2::XMLElement * findFirstXMLElementWithClassNames(const tinyxml2::XMLElement * element, const std::vector<std::string> & classNames);
	const tinyxml2::XMLElement * findFirstXMLElementWithFullText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive = false);
	const tinyxml2::XMLElement * findFirstXMLElementContainingText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive = false);
	std::vector<const tinyxml2::XMLElement *> findXMLElementsWithName(const tinyxml2::XMLElement * element, const std::string & name, bool caseSensitive = false);
	std::vector<const tinyxml2::XMLElement *> findXMLElementsWithAttributeValue(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue);
	std::vector<const tinyxml2::XMLElement *> findXMLElementsWithClassName(const tinyxml2::XMLElement * element, const std::string & className);
	std::vector<const tinyxml2::XMLElement *> findXMLElementsWithClassNames(const tinyxml2::XMLElement * element, const std::vector<std::string> & classNames);
	std::vector<const tinyxml2::XMLElement *> findXMLElementsWithFullText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive = false);
	std::vector<const tinyxml2::XMLElement *> findXMLElementsContainingText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive = false);
	std::string documentToString(const tinyxml2::XMLDocument * document);
	std::string elementToString(const tinyxml2::XMLElement * element);
	bool saveXMLDocumentToFile(const tinyxml2::XMLDocument * document, const std::string & filePath, bool overwrite = false, bool compact = false, const std::string & indentation = "\t");

}

#endif // _TINYXML2_UTILITIES_H_

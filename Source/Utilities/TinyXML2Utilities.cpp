#include "TinyXML2Utilities.h"

#include "Utilities/StringUtilities.h"

#include <fmt/core.h>

const tinyxml2::XMLElement * Utilities::findXMLElementWithID(const tinyxml2::XMLElement * element, const std::string & id) {
	static constexpr const char * ID_XML_ATTRIBUTE_NAME = "id";

	if(element == nullptr || id.empty()) {
		return nullptr;
	}

	const char * currentElementID = element->Attribute(ID_XML_ATTRIBUTE_NAME);

	if(currentElementID != nullptr && Utilities::areStringsEqual(currentElementID, id)) {
		return element;
	}

	const tinyxml2::XMLElement * matchingChildElement = nullptr;
	const tinyxml2::XMLElement * childElement = element->FirstChildElement();

	while(true) {
		if(childElement == nullptr) {
			break;
		}

		matchingChildElement = findXMLElementWithID(childElement, id);

		if(matchingChildElement != nullptr) {
			return matchingChildElement;
		}

		childElement = childElement->NextSiblingElement();
	}

	return nullptr;
}

const tinyxml2::XMLElement * Utilities::findXMLElementWithName(const tinyxml2::XMLElement * element, const std::string & name) {
	if(element == nullptr || name.empty()) {
		return nullptr;
	}

	if(Utilities::areStringsEqual(element->Name(), name)) {
		return element;
	}

	const tinyxml2::XMLElement * matchingChildElement = nullptr;
	const tinyxml2::XMLElement * childElement = element->FirstChildElement();

	while(true) {
		if(childElement == nullptr) {
			break;
		}

		matchingChildElement = findXMLElementWithName(childElement, name);

		if(matchingChildElement != nullptr) {
			return matchingChildElement;
		}

		childElement = childElement->NextSiblingElement();
	}

	return nullptr;
}

static void findXMLElementsWithNameHelper(const tinyxml2::XMLElement * element, const std::string & name, std::vector<const tinyxml2::XMLElement *> & matchingElements) {
	if(element == nullptr || name.empty()) {
		return;
	}

	if(Utilities::areStringsEqual(element->Name(), name)) {
		matchingElements.push_back(element);
	}

	const tinyxml2::XMLElement * childElement = element->FirstChildElement();

	while(true) {
		if(childElement == nullptr) {
			break;
		}

		findXMLElementsWithNameHelper(childElement, name, matchingElements);

		childElement = childElement->NextSiblingElement();
	}
}

std::vector<const tinyxml2::XMLElement *> Utilities::findXMLElementsWithName(const tinyxml2::XMLElement * element, const std::string & name) {
	std::vector<const tinyxml2::XMLElement *> matchingElements;

	findXMLElementsWithNameHelper(element, name, matchingElements);

	return matchingElements;
}

static void findXMLElementsWithAttributeValueHelper(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue, std::vector<const tinyxml2::XMLElement *> & matchingElements) {
	if(element == nullptr || attributeName.empty()) {
		return;
	}

	const char * currentElementAttributeValue = element->Attribute(attributeName.c_str());

	if(currentElementAttributeValue != nullptr && Utilities::areStringsEqual(currentElementAttributeValue, attributeValue)) {
		matchingElements.push_back(element);
	}

	const tinyxml2::XMLElement * childElement = element->FirstChildElement();

	while(true) {
		if(childElement == nullptr) {
			break;
		}

		findXMLElementsWithAttributeValueHelper(childElement, attributeName, attributeValue, matchingElements);

		childElement = childElement->NextSiblingElement();
	}
}

std::vector<const tinyxml2::XMLElement *> Utilities::findXMLElementsWithAttributeValue(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue) {
	std::vector<const tinyxml2::XMLElement *> matchingElements;

	findXMLElementsWithAttributeValueHelper(element, attributeName, attributeValue, matchingElements);

	return matchingElements;
}

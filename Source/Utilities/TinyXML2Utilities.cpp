#include "TinyXML2Utilities.h"

#include "Utilities/StringUtilities.h"
#include "XML/CustomIndentationXMLPrinter.h"

#include <fmt/core.h>

#include <filesystem>
#include <functional>

static bool xmlElementAttributeValueMatcher(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string attributeValue) {
	if(element == nullptr || attributeName.empty()) {
		return false;
	}

	const char * elementID = element->Attribute(attributeName.c_str());

	return elementID != nullptr && Utilities::areStringsEqual(elementID, attributeValue);
}

static bool xmlElementIDMatcher(const tinyxml2::XMLElement * element, const std::string & id) {
	static constexpr const char * XML_ID_ATTRIBUTE_NAME = "id";

	return xmlElementAttributeValueMatcher(element, XML_ID_ATTRIBUTE_NAME, id);
}

static bool xmlElementNameMatcher(const tinyxml2::XMLElement * element, const std::string & name, bool caseSensitive = false) {
	if(element == nullptr || name.empty()) {
		return false;
	}

	return Utilities::areStringsEqual(element->Name(), name, caseSensitive);
}

static bool xmlElementClassNameExtractor(const tinyxml2::XMLElement * element, const std::string & className, std::vector<std::string> * elementClassNames = nullptr) {
	static constexpr const char * XML_CLASS_ATTRIBUTE_NAME = "class";

	if(element == nullptr) {
		return false;
	}

	const char * elementClassesRaw = element->Attribute(XML_CLASS_ATTRIBUTE_NAME);

	if(elementClassesRaw == nullptr) {
		return false;
	}

	bool classNameMatches = false;
	size_t currentClassNameSeparatorIndex = 0;
	size_t previousClassNameSeparatorIndex = std::string::npos;
	std::string_view currentClassName;
	std::string_view elementClasses(elementClassesRaw);

	while(true) {
		currentClassNameSeparatorIndex = elementClasses.find_first_of(" \t", previousClassNameSeparatorIndex == std::string::npos ? 0 : previousClassNameSeparatorIndex + 1);

		if(currentClassNameSeparatorIndex == std::string::npos) {
			size_t classNameStartIndex = elementClasses.find_first_not_of(" \t", previousClassNameSeparatorIndex);

			if(classNameStartIndex != std::string::npos) {
				currentClassNameSeparatorIndex = classNameStartIndex - 1;
			}
			else {
				classNameStartIndex = previousClassNameSeparatorIndex;
			}

			currentClassName = std::string_view(elementClasses.data() + (classNameStartIndex == std::string::npos ? 0 : classNameStartIndex), elementClasses.length() - (classNameStartIndex == std::string::npos ? 0 : classNameStartIndex));

			size_t currentClassNameEndIndex = currentClassName.find_last_not_of(" \t");

			if(currentClassNameEndIndex == std::string::npos) {
				break;
			}

			currentClassName = std::string_view(currentClassName.data(), currentClassNameEndIndex + 1);

			if(!currentClassName.empty()) {
				if(elementClassNames != nullptr) {
					elementClassNames->emplace_back(currentClassName);
				}

				if(!className.empty()) {
					if(Utilities::areStringsEqual(className, currentClassName)) {
						classNameMatches = true;
					}

					if(elementClassNames == nullptr && classNameMatches) {
						return true;
					}
				}
			}

			break;
		}

		currentClassName = std::string_view(elementClasses.data() + (previousClassNameSeparatorIndex == std::string::npos ? 0 : previousClassNameSeparatorIndex + 1), currentClassNameSeparatorIndex - (previousClassNameSeparatorIndex == std::string::npos ? 0 : previousClassNameSeparatorIndex + 1));

		if(!currentClassName.empty()) {
			if(elementClassNames != nullptr) {
				elementClassNames->emplace_back(currentClassName);
			}

			if(!className.empty()) {
				if(Utilities::areStringsEqual(className, currentClassName)) {
					classNameMatches = true;
				}

				if(elementClassNames == nullptr && classNameMatches) {
					return true;
				}
			}
		}

		previousClassNameSeparatorIndex = currentClassNameSeparatorIndex;
	}

	return classNameMatches;
}

static bool xmlElementClassNameMatcher(const tinyxml2::XMLElement * element, const std::string & className) {
	return xmlElementClassNameExtractor(element, className, nullptr);
}

static bool xmlElementClassNamesMatcher(const tinyxml2::XMLElement * element, const std::vector<std::string> & classNames) {
	if(element == nullptr || classNames.empty()) {
		return false;
	}

	std::vector<std::string> elementClassNames;

	xmlElementClassNameExtractor(element, Utilities::emptyString, &elementClassNames);

	for(const std::string & className : classNames) {
		if(std::find_if(elementClassNames.cbegin(), elementClassNames.cend(), [&className](const std::string & currentElementClassName) {
			return Utilities::areStringsEqual(className, currentElementClassName);	
		}) == elementClassNames.cend()) {
			return false;
		}
	}

	return true;
}

static bool xmlElementFullTextMatcher(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive) {
	if(element == nullptr || text.empty()) {
		return false;
	}

	const char * elementTextRaw = element->GetText();

	if(elementTextRaw == nullptr) {
		return false;
	}

	std::string_view elementText(elementTextRaw);

	size_t textStartIndex = elementText.find_first_not_of(" \t");
	size_t textEndIndex = elementText.find_last_not_of(" \t");

	if(textStartIndex == std::string::npos || textEndIndex == std::string::npos) {
		return false;
	}

	std::string_view trimmedElementText(elementText.data() + textStartIndex, textEndIndex - textStartIndex + 1);

	return Utilities::areStringsEqual(trimmedElementText, text, caseSensitive);
}

static bool xmlElementContainsTextMatcher(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive) {
	if(element == nullptr || text.empty()) {
		return false;
	}

	const char * elementTextRaw = element->GetText();

	if(elementTextRaw == nullptr) {
		return false;
	}

	std::string_view elementText(elementTextRaw);

	if(caseSensitive) {
		return elementText.find(text) != std::string::npos;
	}
	else {
		return Utilities::toLowerCase(elementText).find(Utilities::toLowerCase(text)) != std::string::npos;
	}
}

static const tinyxml2::XMLElement * findFirstMatchingXMLElementHelper(const tinyxml2::XMLElement * element, std::function<bool(const tinyxml2::XMLElement *)> matchFunction) {
	if(element == nullptr || matchFunction == nullptr) {
		return nullptr;
	}

	if(matchFunction(element)) {
		return element;
	}

	const tinyxml2::XMLElement * matchingChildElement = nullptr;
	const tinyxml2::XMLElement * childElement = element->FirstChildElement();

	while(true) {
		if(childElement == nullptr) {
			break;
		}

		matchingChildElement = findFirstMatchingXMLElementHelper(childElement, matchFunction);

		if(matchingChildElement != nullptr) {
			return matchingChildElement;
		}

		childElement = childElement->NextSiblingElement();
	}

	return nullptr;
}

static void findMatchingXMLElementsHelper(const tinyxml2::XMLElement * element, std::function<bool(const tinyxml2::XMLElement *)> matchFunction, std::vector<const tinyxml2::XMLElement *> & matchingElements) {
	if(element == nullptr || matchFunction == nullptr) {
		return;
	}

	if(matchFunction(element)) {
		matchingElements.push_back(element);
	}

	const tinyxml2::XMLElement * childElement = element->FirstChildElement();

	while(true) {
		if(childElement == nullptr) {
			break;
		}

		findMatchingXMLElementsHelper(childElement, matchFunction, matchingElements);

		childElement = childElement->NextSiblingElement();
	}
}

bool Utilities::doesXMLElementHaveID(const tinyxml2::XMLElement * element, const std::string & id) {
	return xmlElementIDMatcher(element, id);
}

bool Utilities::doesXMLElementHaveAttributeValue(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue) {
	return xmlElementAttributeValueMatcher(element, attributeName, attributeValue);
}

bool Utilities::doesXMLElementHaveClassName(const tinyxml2::XMLElement * element, const std::string & className) {
	return xmlElementClassNameMatcher(element, className);
}

bool Utilities::doesXMLElementHaveClassNames(const tinyxml2::XMLElement * element, const std::vector<std::string> & classNames) {
	return xmlElementClassNamesMatcher(element, classNames);
}

bool Utilities::doesXMLElementHaveFullText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive) {
	return xmlElementFullTextMatcher(element, text, caseSensitive);
}

bool Utilities::doesXMLElementContainText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive) {
	return xmlElementContainsTextMatcher(element, text, caseSensitive);
}

const tinyxml2::XMLElement * Utilities::getParentXMLElement(const tinyxml2::XMLElement * element) {
	if(element == nullptr) {
		return nullptr;
	}

	const tinyxml2::XMLNode * parentNode = element->Parent();

	if(parentNode == nullptr) {
		return nullptr;
	}

	return parentNode->ToElement();
}

std::vector<std::string> Utilities::getXMLElementClassNames(const tinyxml2::XMLElement * element) {
	std::vector<std::string> elementClassNames;

	xmlElementClassNameExtractor(element, Utilities::emptyString, &elementClassNames);

	return elementClassNames;
}

const tinyxml2::XMLElement * Utilities::findXMLElementWithID(const tinyxml2::XMLElement * element, const std::string & id) {
	if(element == nullptr || id.empty()) {
		return nullptr;
	}

	return findFirstMatchingXMLElementHelper(element, std::bind(&xmlElementIDMatcher, std::placeholders::_1, id));
}

const tinyxml2::XMLElement * Utilities::findFirstXMLElementWithName(const tinyxml2::XMLElement * element, const std::string & name, bool caseSensitive) {
	if(element == nullptr || name.empty()) {
		return nullptr;
	}

	return findFirstMatchingXMLElementHelper(element, std::bind(&xmlElementNameMatcher, std::placeholders::_1, name, caseSensitive));
}

const tinyxml2::XMLElement * Utilities::findFirstXMLElementWithAttributeValue(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue) {
	if(element == nullptr || attributeName.empty()) {
		return nullptr;
	}

	return findFirstMatchingXMLElementHelper(element, std::bind(&xmlElementAttributeValueMatcher, std::placeholders::_1, attributeName, attributeValue));
}

const tinyxml2::XMLElement * Utilities::findFirstXMLElementWithClassName(const tinyxml2::XMLElement * element, const std::string & className) {
	if(element == nullptr || className.empty()) {
		return false;
	}

	return findFirstMatchingXMLElementHelper(element, std::bind(&xmlElementClassNameMatcher, std::placeholders::_1, className));
}

const tinyxml2::XMLElement * Utilities::findFirstXMLElementWithClassNames(const tinyxml2::XMLElement * element, const std::vector<std::string> & classNames) {
	if(element == nullptr || classNames.empty()) {
		return false;
	}

	return findFirstMatchingXMLElementHelper(element, std::bind(&xmlElementClassNamesMatcher, std::placeholders::_1, classNames));
}

const tinyxml2::XMLElement * Utilities::findFirstXMLElementWithFullText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive) {
	if(element == nullptr || text.empty()) {
		return false;
	}

	return findFirstMatchingXMLElementHelper(element, std::bind(&xmlElementFullTextMatcher, std::placeholders::_1, text, caseSensitive));

}

const tinyxml2::XMLElement * Utilities::findFirstXMLElementContainingText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive) {
	if(element == nullptr || text.empty()) {
		return false;
	}

	return findFirstMatchingXMLElementHelper(element, std::bind(&xmlElementContainsTextMatcher, std::placeholders::_1, text, caseSensitive));
}

std::vector<const tinyxml2::XMLElement *> Utilities::findXMLElementsWithName(const tinyxml2::XMLElement * element, const std::string & name, bool caseSensitive) {
	if(element == nullptr || name.empty()) {
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> matchingElements;

	findMatchingXMLElementsHelper(element, std::bind(&xmlElementNameMatcher, std::placeholders::_1, name, caseSensitive), matchingElements);

	return matchingElements;
}

std::vector<const tinyxml2::XMLElement *> Utilities::findXMLElementsWithAttributeValue(const tinyxml2::XMLElement * element, const std::string & attributeName, const std::string & attributeValue) {
	if(element == nullptr || attributeName.empty()) {
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> matchingElements;

	findMatchingXMLElementsHelper(element, std::bind(&xmlElementAttributeValueMatcher, std::placeholders::_1, attributeName, attributeValue), matchingElements);

	return matchingElements;
}

std::vector<const tinyxml2::XMLElement *> Utilities::findXMLElementsWithClassName(const tinyxml2::XMLElement * element, const std::string & className) {
	if(element == nullptr || className.empty()) {
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> matchingElements;

	findMatchingXMLElementsHelper(element, std::bind(&xmlElementClassNameMatcher, std::placeholders::_1, className), matchingElements);

	return matchingElements;
}

std::vector<const tinyxml2::XMLElement *> Utilities::findXMLElementsWithClassNames(const tinyxml2::XMLElement * element, const std::vector<std::string> & classNames) {
	if(element == nullptr || classNames.empty()) {
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> matchingElements;

	findMatchingXMLElementsHelper(element, std::bind(&xmlElementClassNamesMatcher, std::placeholders::_1, classNames), matchingElements);

	return matchingElements;
}

std::vector<const tinyxml2::XMLElement *> Utilities::findXMLElementsWithFullText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive) {
	if(element == nullptr || text.empty()) {
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> matchingElements;

	findMatchingXMLElementsHelper(element, std::bind(&xmlElementFullTextMatcher, std::placeholders::_1, text, caseSensitive), matchingElements);

	return matchingElements;
}

std::vector<const tinyxml2::XMLElement *> Utilities::findXMLElementsContainingText(const tinyxml2::XMLElement * element, const std::string & text, bool caseSensitive) {
	if(element == nullptr || text.empty()) {
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> matchingElements;

	findMatchingXMLElementsHelper(element, std::bind(&xmlElementContainsTextMatcher, std::placeholders::_1, text, caseSensitive), matchingElements);

	return matchingElements;
}

std::string Utilities::documentToString(const tinyxml2::XMLDocument * document) {
	if(document == nullptr) {
		return {};
	}

	return elementToString(document->RootElement());
}

std::string Utilities::elementToString(const tinyxml2::XMLElement * element) {
	if(element == nullptr) {
		return {};
	}

	CustomIndentationXMLPrinter xmlPrinter;
	element->Accept(&xmlPrinter);

	return std::string(xmlPrinter.CStr(), xmlPrinter.CStrSize());
}

bool Utilities::saveXMLDocumentToFile(const tinyxml2::XMLDocument * document, const std::string & filePath, bool overwrite, bool compact, const std::string & indentation) {
	if(document == nullptr || filePath.empty() || (!overwrite && std::filesystem::is_regular_file(std::filesystem::path(filePath)))) {
		return false;
	}

	FILE * file = nullptr;

#if defined(_MSC_VER) && (_MSC_VER >= 1400 ) && (!defined WINCE)
	const errno_t error = fopen_s(&file, filePath.c_str(), "wb");

	if(error) {
		return false;
	}
#else
	file = fopen(filePath.c_str(), "wb");
#endif

	if(file == nullptr) {
		return false;
	}

	CustomIndentationXMLPrinter printer(file, compact, 0, indentation);

	document->Print(&printer);

	fclose(file);

	return true;
}

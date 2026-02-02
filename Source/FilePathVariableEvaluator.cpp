#include "FilePathVariableEvaluator.h"

#include "Platform/SystemInformationBridge.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <regex>
#include <sstream>

bool FilePathVariableEvaluator::NameComparator::operator () (const std::string & nameA, const std::string & nameB) const {
	return std::lexicographical_compare(nameA.begin(), nameA.end(), nameB.begin(), nameB.end(), [](unsigned char a, unsigned char b) {
		return std::tolower(a) < std::tolower(b);
	});
}

FilePathVariableEvaluator::FilePathVariableEvaluator() {
	SystemInformationBridge * systemInformationBridge = SystemInformationBridge::getInstance();

	if(systemInformationBridge) {
		std::optional<std::string> optionalHomeDirectoryPath(systemInformationBridge->getHomeDirectoryPath());

		if(optionalHomeDirectoryPath.has_value()) {
			m_defaultVariables.emplace("HOME", optionalHomeDirectoryPath.value());
			m_defaultVariables.emplace("USERPROFILE", optionalHomeDirectoryPath.value());
		}

		std::optional<std::string> optionalApplicationDataDirectoryPath(systemInformationBridge->getApplicationDataDirectoryPath());

		if(optionalApplicationDataDirectoryPath.has_value()) {
			m_defaultVariables.emplace("APPDATA", optionalApplicationDataDirectoryPath.value());
		}
	}
}

std::optional<std::string> FilePathVariableEvaluator::evaluateFilePath(const std::string & filePath, const VariableMap & customVariables, char variableStartChar, char variableEndChar) const {
	const std::regex variableRegExp(fmt::format("{0}[^{0}{1}]+{1}", variableStartChar, variableEndChar));

	if(filePath.empty()) {
		return "";
	}

	std::vector<std::string> directoryPathParts(Utilities::regularExpressionStringSplit(filePath, variableRegExp));
	std::vector<std::string> directoryPathVariableNames;
	std::string directoryPathSearchString = filePath;
	std::smatch directoryPathVariableMatch;

	while(std::regex_search(directoryPathSearchString, directoryPathVariableMatch, variableRegExp)) {
		for(std::sub_match submatch : directoryPathVariableMatch) {
			std::string variableName(submatch.str());
			directoryPathVariableNames.push_back(variableName.substr(1, variableName.length() - 2));
		}

		directoryPathSearchString = directoryPathVariableMatch.suffix().str();
	}

	bool variableFirst = directoryPathParts.empty() || filePath.find(directoryPathParts[0]) != 0;

	std::optional<std::string> optionalVariableValue;
	std::stringstream newDirectoryPath;
	size_t j = 0;

	for(size_t i = 0; i < directoryPathParts.size(); i++) {
		if(!variableFirst) {
			newDirectoryPath << directoryPathParts[i];
		}

		if(j < directoryPathVariableNames.size()) {
			const std::string & variableName = directoryPathVariableNames[j];
			optionalVariableValue.reset();

			VariableMap::const_iterator customVariableIterator(customVariables.find(variableName));

			if(customVariableIterator != customVariables.cend()) {
				optionalVariableValue = customVariableIterator->second;
			}
			else {
				VariableMap::const_iterator defaultVariableIterator(m_defaultVariables.find(variableName));

				if(defaultVariableIterator != m_defaultVariables.cend()) {
					optionalVariableValue = defaultVariableIterator->second;
				}
			}

			if(optionalVariableValue.has_value()) {
				newDirectoryPath << optionalVariableValue.value();
			}
			else {
				const char * environmentVariableValue = std::getenv(variableName.c_str());

				if(environmentVariableValue != nullptr) {
					newDirectoryPath << environmentVariableValue;
				}
				else {
					spdlog::error("Failed to evaluate file path with unknown variable: '{}{}{}'.", variableStartChar, variableName, variableEndChar);

					return {};
				}
			}

			j++;
		}

		if(variableFirst) {
			newDirectoryPath << directoryPathParts[i];
		}
	}

	return newDirectoryPath.str();
}

#include "ArgumentCollection.h"

#include "Utilities/StringUtilities.h"

#include <sstream>

const ArgumentCollection::ArgumentCase ArgumentCollection::DEFAULT_CASE = ArgumentCollection::ArgumentCase::OriginalCase;

ArgumentCollection::ArgumentCollection(ArgumentCollection::ArgumentCase caseType)
	: m_case(caseType) { }

ArgumentCollection::ArgumentCollection(std::vector<std::pair<std::string, std::string>> arguments, ArgumentCase caseType)
	: m_case(caseType) {
	for(const std::pair<std::string, std::string> & argument : arguments) {
		addArgument(argument.first, argument.second);
	}
}

ArgumentCollection::ArgumentCollection(std::map<std::string, std::string> arguments, ArgumentCase caseType)
	: m_case(caseType) {
	for(std::map<std::string, std::string>::const_iterator i = arguments.begin(); i != arguments.end(); ++i) {
		m_arguments.insert(std::pair<std::string, std::string>(i->first, i->second));
	}
}

ArgumentCollection::ArgumentCollection(std::multimap<std::string, std::string> arguments, ArgumentCase caseType)
	: m_case(caseType) {
	for(std::multimap<std::string, std::string>::const_iterator i = arguments.begin(); i != arguments.end(); ++i) {
		m_arguments.insert(std::pair<std::string, std::string>(i->first, i->second));
	}
}

ArgumentCollection::ArgumentCollection(const ArgumentCollection & argumentCollection) {
	m_case = argumentCollection.m_case;

	for(std::multimap<std::string, std::string>::const_iterator i = argumentCollection.m_arguments.begin(); i != argumentCollection.m_arguments.end(); ++i) {
		m_arguments.insert(std::pair<std::string, std::string>(i->first, i->second));
	}
}

ArgumentCollection::ArgumentCollection(ArgumentCollection && argumentCollection) noexcept
	: m_case(argumentCollection.m_case)
	, m_arguments(std::move(argumentCollection.m_arguments)) { }

ArgumentCollection & ArgumentCollection::operator = (ArgumentCollection && argumentCollection) noexcept {
	if (this != &argumentCollection) {
		m_case = argumentCollection.m_case;
		m_arguments = std::move(argumentCollection.m_arguments);
	}

	return *this;
}

ArgumentCollection & ArgumentCollection::operator = (const ArgumentCollection & argumentCollection) {
	m_arguments.clear();

	m_case = argumentCollection.m_case;

	for(std::multimap<std::string, std::string>::const_iterator i = argumentCollection.m_arguments.begin(); i != argumentCollection.m_arguments.end(); ++i) {
		m_arguments.insert(std::pair<std::string, std::string>(i->first, i->second));
	}

	return *this;
}

ArgumentCollection::~ArgumentCollection() { }

ArgumentCollection::ArgumentCase ArgumentCollection::getCase() const {
	return m_case;
}

void ArgumentCollection::setCase(ArgumentCollection::ArgumentCase caseType) {
	m_case = caseType;
}

size_t ArgumentCollection::numberOfArguments() const {
	return m_arguments.size();
}

size_t ArgumentCollection::numberOfArguments(const std::string & name) const {
	std::string formattedArgumentName(formatArgumentName(name));

	if(formattedArgumentName.empty()) {
		return 0;
	}

	return m_arguments.count(formattedArgumentName);
}

bool ArgumentCollection::hasArgument(const std::string & name) const {
	std::string formattedArgumentName(formatArgumentName(name));

	if(formattedArgumentName.empty()) {
		return false;
	}

	return m_arguments.find(formattedArgumentName) != m_arguments.end();
}

std::string ArgumentCollection::getFirstValue(const std::string & name) const {
	std::string formattedArgumentName(formatArgumentName(name));

	if(formattedArgumentName.empty()) {
		return {};
	}

	std::multimap<std::string, std::string>::const_iterator argumentIterator(m_arguments.find(formattedArgumentName));

	if(argumentIterator == m_arguments.end()) {
		return {};
	}

	return argumentIterator->second;
}

std::string ArgumentCollection::getLastValue(const std::string & name) const {
	std::string formattedArgumentName(formatArgumentName(name));

	if(formattedArgumentName.empty()) {
		return {};
	}

	std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator> argumentRange(m_arguments.equal_range(formattedArgumentName));

	if(argumentRange.first == argumentRange.second) {
		return {};
	}

	std::multimap<std::string, std::string>::const_iterator lastArgumentIterator(argumentRange.second);
	lastArgumentIterator--;

	return lastArgumentIterator->second;
}

std::vector<std::string> ArgumentCollection::getValues(const std::string & name) const {
	std::string formattedArgumentName(formatArgumentName(name));

	if(formattedArgumentName.empty()) {
		return {};
	}

	std::vector<std::string> values;
	std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator> argumentRange(m_arguments.equal_range(formattedArgumentName));

	for(std::multimap<std::string, std::string>::const_iterator i = argumentRange.first; i != argumentRange.second; ++i) {
		values.emplace_back(i->second);
	}

	return values;
}

bool ArgumentCollection::addArgument(const std::string & name, const std::string & value) {
	std::string formattedArgumentName(formatArgumentName(name));

	if(formattedArgumentName.empty()) {
		return false;
	}

	m_arguments.insert(std::pair<std::string, std::string>(formattedArgumentName, value));

	return true;
}

void ArgumentCollection::removeArgument(const std::string & name) {
	std::string formattedArgumentName(formatArgumentName(name));

	if(formattedArgumentName.empty()) {
		return;
	}

	m_arguments.erase(formattedArgumentName);
}

void ArgumentCollection::clear() {
	m_arguments.clear();
}

std::string ArgumentCollection::formatArgumentName(std::string_view data) const {
	if(data.empty()) {
		return {};
	}

	std::string formattedData(Utilities::trimString(data));

	if(m_case == ArgumentCollection::ArgumentCase::UpperCase) {
		return Utilities::toUpperCase(formattedData);
	}
	else if(m_case == ArgumentCollection::ArgumentCase::LowerCase) {
		return Utilities::toLowerCase(formattedData);
	}

	return formattedData;
}

std::string ArgumentCollection::toString() const {
	std::stringstream args;

	for(std::multimap<std::string, std::string>::const_iterator i = m_arguments.begin(); i != m_arguments.end(); ++i) {
		if(i != m_arguments.begin()) {
			args << " ";
		}

		args << (i->first.length() == 1 ? "-" : "--") << i->first;

		if(!i->second.empty()) {
			args << " " << i->second;
		}
	}

	return args.str();
}

bool ArgumentCollection::operator == (const ArgumentCollection & argumentCollection) const {
	if(m_arguments.size() != argumentCollection.m_arguments.size()) {
		return false;
	}

	for(std::multimap<std::string, std::string>::const_iterator i = m_arguments.begin(); i != m_arguments.end(); ++i) {
		std::multimap<std::string, std::string>::const_iterator argumentIterator(argumentCollection.m_arguments.find(i->first));

		if(argumentIterator == m_arguments.end()) {
			return false;
		}

		std::string key(i->first);

		if(i->second != argumentIterator->second) {
			return false;
		}
	}
	return true;
}

bool ArgumentCollection::operator != (const ArgumentCollection & s) const {
	return !operator == (s);
}

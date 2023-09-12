#ifndef _ARGUMENT_COLLECTION_H_
#define _ARGUMENT_COLLECTION_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class ArgumentCollection {
public:
	enum class ArgumentCase {
		OriginalCase,
		UpperCase,
		LowerCase
	};

	ArgumentCollection(ArgumentCase caseType = DEFAULT_CASE);
	ArgumentCollection(std::vector<std::pair<std::string, std::string>> arguments, ArgumentCase caseType = DEFAULT_CASE);
	template <size_t N>
	ArgumentCollection(std::array<std::pair<std::string, std::string>, N> arguments, ArgumentCase caseType = DEFAULT_CASE);
	ArgumentCollection(std::map<std::string, std::string> arguments, ArgumentCase caseType = DEFAULT_CASE);
	ArgumentCollection(std::multimap<std::string, std::string> arguments, ArgumentCase caseType = DEFAULT_CASE);
	ArgumentCollection(ArgumentCollection && argumentCollection) noexcept;
	ArgumentCollection(const ArgumentCollection & argumentCollection);
	ArgumentCollection & operator = (ArgumentCollection && argumentCollection) noexcept;
	ArgumentCollection & operator = (const ArgumentCollection & argumentCollection);
	virtual ~ArgumentCollection();

	ArgumentCase getCase() const;
	void setCase(ArgumentCase caseType);

	size_t numberOfArguments() const;
	size_t numberOfArguments(const std::string & name) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	size_t numberOfArguments(Arguments &&... arguments) const;
	bool hasArgument(const std::string & name) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	bool hasArgument(Arguments &&... arguments) const;
	std::string getFirstValue(const std::string & name) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::string getFirstValue(Arguments &&... arguments) const;
	std::string getLastValue(const std::string & name) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::string getLastValue(Arguments &&... arguments) const;
	std::vector<std::string> getValues(const std::string & name) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::vector<std::string> getValues(Arguments &&... arguments) const;
	bool addArgument(const std::string & name, const std::string & value);
	void removeArgument(const std::string & name);
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	void removeArgument(Arguments &&... arguments);
	void clear();

	std::string toString() const;

	bool operator == (const ArgumentCollection & argumentCollection) const;
	bool operator != (const ArgumentCollection & argumentCollection) const;

	static const ArgumentCase DEFAULT_CASE;

protected:
	std::string formatArgumentName(std::string_view data) const;

	ArgumentCase m_case;
	std::multimap<std::string, std::string> m_arguments;
};

template <size_t N>
ArgumentCollection::ArgumentCollection(std::array<std::pair<std::string, std::string>, N> arguments, ArgumentCase caseType)
	: m_case(caseType) {
	for(const std::pair<std::string, std::string> & argument : arguments) {
		addArgument(argument.first, argument.second);
	}
}

template <typename ...Arguments, typename>
size_t ArgumentCollection::numberOfArguments(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};
	size_t totalArgumentCount = 0;

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		std::string formattedArgumentName(formatArgumentName(unpackedArguments[i]));

		if(formattedArgumentName.empty()) {
			continue;
		}

		totalArgumentCount += m_arguments.count(formattedArgumentName);
	}

	return totalArgumentCount;
}

template <typename ...Arguments, typename>
bool ArgumentCollection::hasArgument(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		std::string formattedArgumentName(formatArgumentName(unpackedArguments[i]));

		if(!formattedArgumentName.empty() && m_arguments.find(formattedArgumentName) != m_arguments.end()) {
			return true;
		}
	}

	return false;
}

template <typename ...Arguments, typename>
std::string ArgumentCollection::getFirstValue(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		std::string formattedArgumentName(formatArgumentName(unpackedArguments[i]));

		if(formattedArgumentName.empty()) {
			continue;
		}

		std::multimap<std::string, std::string>::const_iterator argumentIterator(m_arguments.find(formattedArgumentName));

		if(argumentIterator == m_arguments.end()) {
			continue;
		}

		return argumentIterator->second;
	}

	return {};
}

template <typename ...Arguments, typename>
std::string ArgumentCollection::getLastValue(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		std::string formattedArgumentName(formatArgumentName(unpackedArguments[i]));

		if(formattedArgumentName.empty()) {
			continue;
		}

		std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator> argumentRange(m_arguments.equal_range(formattedArgumentName));

		if(argumentRange.first == argumentRange.second) {
			continue;
		}

		std::multimap<std::string, std::string>::const_iterator lastArgumentIterator(argumentRange.second);
		lastArgumentIterator--;

		return lastArgumentIterator->second;
	}

	return {};
}

template <typename ...Arguments, typename>
std::vector<std::string> ArgumentCollection::getValues(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};
	std::vector<std::string> values;

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		std::string formattedArgumentName(formatArgumentName(unpackedArguments[i]));

		if(formattedArgumentName.empty()) {
			continue;
		}

		std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator> argumentRange(m_arguments.equal_range(formattedArgumentName));

		for(std::multimap<std::string, std::string>::const_iterator i = argumentRange.first; i != argumentRange.second; ++i) {
			values.emplace_back(i->second);
		}
	}

	return values;
}

template <typename ...Arguments, typename>
void ArgumentCollection::removeArgument(Arguments &&... arguments) {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		std::string formattedArgumentName(formatArgumentName(unpackedArguments[i]));

		if(formattedArgumentName.empty()) {
			continue;
		}

		m_arguments.erase(formattedArgumentName);
	}
}

#endif // _ARGUMENT_COLLECTION_H_

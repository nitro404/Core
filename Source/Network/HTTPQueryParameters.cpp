#include "HTTPQueryParameters.h"

#include <iomanip>
#include <sstream>

static constexpr char QUERY_STRING_DELIMITER = '?';
static constexpr char QUERY_PARAMETER_DELIMITER = '&';
static constexpr char QUERY_PARAMETER_VALUE_DELIMITER = '=';

HTTPQueryParameters::HTTPQueryParameters(const QueryParameterMap & queryParameters)
	: m_queryParameters(queryParameters) { }

HTTPQueryParameters::HTTPQueryParameters(QueryParameterMap && queryParameters)
	: m_queryParameters(std::move(queryParameters)) { }

HTTPQueryParameters::HTTPQueryParameters(HTTPQueryParameters && queryParameters) noexcept
	: m_queryParameters(std::move(queryParameters.m_queryParameters)) { }

HTTPQueryParameters::HTTPQueryParameters(const HTTPQueryParameters & queryParameters)
	: m_queryParameters(queryParameters.m_queryParameters) { }

HTTPQueryParameters & HTTPQueryParameters::operator = (HTTPQueryParameters && queryParameters) noexcept {
	if(this != &queryParameters) {
		m_queryParameters = std::move(queryParameters.m_queryParameters);
	}

	return *this;
}

HTTPQueryParameters & HTTPQueryParameters::operator = (const HTTPQueryParameters & queryParameters) {
	if(this != &queryParameters) {
		m_queryParameters = queryParameters.m_queryParameters;
	}

	return *this;
}

HTTPQueryParameters::~HTTPQueryParameters() { }

bool HTTPQueryParameters::isEmpty() const {
	return m_queryParameters.empty();
}

bool HTTPQueryParameters::isNotEmpty() const {
	return !m_queryParameters.empty();
}

size_t HTTPQueryParameters::numberOfQueryParameters() const {
	return m_queryParameters.size();
}

bool HTTPQueryParameters::hasKey(std::string_view key) const {
	return m_queryParameters.find(key) != m_queryParameters.end();
}

std::vector<std::string> HTTPQueryParameters::getKeys() const {
	std::vector<std::string> keys;

	for(QueryParameterMap::const_iterator i = m_queryParameters.cbegin(); i != m_queryParameters.cend(); ++i) {
		keys.push_back(i->first);
	}

	return keys;
}

std::string HTTPQueryParameters::getFirstValue(std::string_view key) const {
	QueryParameterMap::const_iterator queryParameterIterator(m_queryParameters.find(key));

	if(queryParameterIterator == m_queryParameters.end()) {
		return {};
	}

	return queryParameterIterator->second;
}

std::string HTTPQueryParameters::getLastValue(std::string_view key) const {
	std::pair<QueryParameterMap::const_iterator, QueryParameterMap::const_iterator> queryParameterRange(m_queryParameters.equal_range(key));

	if(queryParameterRange.first == queryParameterRange.second) {
		return {};
	}

	QueryParameterMap::const_iterator lastQueryParameterIterator(queryParameterRange.second);
	lastQueryParameterIterator--;

	return lastQueryParameterIterator->second;
}

std::vector<std::string> HTTPQueryParameters::getValues(std::string_view key) const {
	std::vector<std::string> values;
	std::pair<QueryParameterMap::const_iterator, QueryParameterMap::const_iterator> queryParameterRange(m_queryParameters.equal_range(key));

	for(QueryParameterMap::const_iterator i = queryParameterRange.first; i != queryParameterRange.second; ++i) {
		values.emplace_back(i->second);
	}

	return values;
}

const HTTPQueryParameters::QueryParameterMap & HTTPQueryParameters::getQueryParameters() const {
	return m_queryParameters;
}

void HTTPQueryParameters::addValue(std::string_view key, std::string_view value) {
	m_queryParameters.insert(std::make_pair(key, value));
}

void HTTPQueryParameters::addValues(std::string_view key, const std::vector<std::string> values) {
	for(const std::string & value : values) {
		m_queryParameters.insert(std::make_pair(key, value));
	}
}

void HTTPQueryParameters::setValue(const std::string & key, std::string_view value) {
	m_queryParameters.erase(key);
	m_queryParameters.insert(std::make_pair(key, value));
}

void HTTPQueryParameters::setValues(const std::string & key, const std::vector<std::string> values) {
	m_queryParameters.erase(key);

	for(const std::string & value : values) {
		m_queryParameters.insert(std::make_pair(key, value));
	}
}

void HTTPQueryParameters::removeKey(const std::string & key) {
	m_queryParameters.erase(key);
}

void HTTPQueryParameters::clear() {
	m_queryParameters.clear();
}

std::string HTTPQueryParameters::toString(bool includeQueryStringDelimiter) const {
	if(isEmpty()) {
		return "";
	}

	std::stringstream queryParameterString;

	if(includeQueryStringDelimiter) {
		queryParameterString << QUERY_STRING_DELIMITER;
	}

	for(QueryParameterMap::const_iterator i = m_queryParameters.begin(); i != m_queryParameters.end(); ++i) {
		if(i != m_queryParameters.begin()) {
			queryParameterString << QUERY_PARAMETER_DELIMITER;
		}

		queryParameterString << encodeData(i->first) << QUERY_PARAMETER_VALUE_DELIMITER << encodeData(i->second);
	}

	return queryParameterString.str();
}

std::string HTTPQueryParameters::encodeData(std::string_view data) {
	char c = '\0';
	bool encode = false;
	const size_t dataLength = data.length();
	std::ostringstream encodedDataStream;

	for(size_t i = 0; i < dataLength; i++) {
		c = data[i];
		encode = true;

		if(std::isalnum(c)) {
			encode = false;
		}

		if(encode) {
			switch(c) {
				case '-':
				case '.':
				case '_':
				case '~':
					encode = false;
					break;
			}
		}

		if(encode) {
			encodedDataStream << '%' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint16_t>(c);
		}
		else {
			encodedDataStream << c;
		}
	}

	return encodedDataStream.str();
}

std::string HTTPQueryParameters::decodeData(std::string_view data) {
	char c = '\0';
	const size_t dataLength = data.length();
	std::ostringstream decodedDataStream;

	for(size_t i = 0; i < dataLength; i++) {
		c = data[i];

		switch(c) {
			case '%': {
				if(i + 2 >= dataLength) {
					continue;
				}

				decodedDataStream << static_cast<char>(std::stoi(std::string(data.data() + i + 1, 2), nullptr, 16));

				i += 2;

				break;
			}
			case '+': {
				decodedDataStream << ' ';
				break;
			}
			default: {
				decodedDataStream << c;
				break;
			}
		}
	}

	return decodedDataStream.str();
}

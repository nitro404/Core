#ifndef _HTTP_QUERY_PARAMETERS_H_
#define _HTTP_QUERY_PARAMETERS_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class HTTPQueryParameters final {
public:
	typedef std::multimap<std::string, std::string, std::less<>> QueryParameterMap;

	HTTPQueryParameters(const QueryParameterMap & queryParameters);
	HTTPQueryParameters(QueryParameterMap && queryParameters = {});
	HTTPQueryParameters(HTTPQueryParameters && queryParameters) noexcept;
	HTTPQueryParameters(const HTTPQueryParameters & queryParameters);
	HTTPQueryParameters & operator = (HTTPQueryParameters && queryParameters) noexcept;
	HTTPQueryParameters & operator = (const HTTPQueryParameters & queryParameters);
	~HTTPQueryParameters();

	bool isEmpty() const;
	bool isNotEmpty() const;
	size_t numberOfQueryParameters() const;
	bool hasKey(std::string_view key) const;
	std::vector<std::string> getKeys() const;
	std::string getFirstValue(std::string_view key) const;
	std::string getLastValue(std::string_view key) const;
	std::vector<std::string> getValues(std::string_view key) const;
	const QueryParameterMap & getQueryParameters() const;
	void addValue(std::string_view key, std::string_view value);
	void addValues(std::string_view key, const std::vector<std::string> values);
	void setValue(const std::string & key, std::string_view value);
	void setValues(const std::string & key, const std::vector<std::string> values);
	void removeKey(const std::string & key);
	void clear();
	std::string toString(bool includeQueryStringDelimiter = true) const;
	static std::string encodeData(std::string_view data);
	static std::string decodeData(std::string_view data);

private:
	std::multimap<std::string, std::string, std::less<>> m_queryParameters;
};

#endif // _HTTP_QUERY_PARAMETERS_H_

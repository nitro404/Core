#ifndef _HTTP_UTILITIES_H_
#define _HTTP_UTILITIES_H_

#include <curl/curl.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace HTTPUtilities {

	using CURLEasyHandle = std::unique_ptr<CURL, std::function<void (CURL *)>>;
	using CURLMultiHandle = std::unique_ptr<CURLM, std::function<void (CURLM *)>>;
	using CURLSharedHandle = std::unique_ptr<CURLSH, std::function<void (CURLSH *)>>;
	using CURLStringList = std::unique_ptr<curl_slist, std::function<void (curl_slist *)>>;

	std::string getStatusCodeName(uint16_t statusCode);
	bool isSuccess(CURLcode code, const std::string & errorMessage = {});
	bool isSuccess(CURLMcode code, const std::string & errorMessage = {});
	bool isSuccess(CURLSHcode code, const std::string & errorMessage = {});
	CURLEasyHandle createCURLEasyHandle();
	CURLMultiHandle createCURLMultiHandle();
	CURLSharedHandle createCURLSharedHandle();
	CURLStringList createCURLStringList();
	bool appendToCURLStringList(CURLStringList & curlStringList, const std::string & string);
	CURLStringList copyCURLStringList(const CURLStringList & curlStringList);

}

#endif // _HTTP_UTILITIES_H_

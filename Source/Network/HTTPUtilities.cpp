#include "HTTPUtilities.h"

#include <fmt/core.h>

bool HTTPUtilities::isSuccess(CURLcode code, const std::string & errorMessage) {
	if(code != CURLE_OK) {
		if(!errorMessage.empty()) {
			fmt::print("{}\n", errorMessage);
		}

		return false;
	}

	return true;
}

bool HTTPUtilities::isSuccess(CURLMcode code, const std::string & errorMessage) {
	if(code != CURLM_OK) {
		if(!errorMessage.empty()) {
			fmt::print("{}\n", errorMessage);
		}

		return false;
	}

	return true;
}

bool HTTPUtilities::isSuccess(CURLSHcode code, const std::string & errorMessage) {
	if(code != CURLSHE_OK) {
		if(!errorMessage.empty()) {
			fmt::print("{}\n", errorMessage);
		}

		return false;
	}

	return true;
}

HTTPUtilities::CURLEasyHandle HTTPUtilities::createCURLEasyHandle() {
	return CURLEasyHandle(curl_easy_init(), [](CURL * curlEasyHandle) {
		curl_easy_cleanup(curlEasyHandle);
	});
}

HTTPUtilities::CURLMultiHandle HTTPUtilities::createCURLMultiHandle() {
	return CURLMultiHandle(curl_multi_init(), [](CURLM * curlMultiHandle) {
		if(!HTTPUtilities::isSuccess(curl_multi_cleanup(curlMultiHandle))) {
			fmt::print("Failed to clean up CURL multi handle.");
		}
	});
}

HTTPUtilities::CURLSharedHandle HTTPUtilities::createCURLSharedHandle() {
	return CURLSharedHandle(curl_share_init(), [](CURLSH * curlSharedHandle) {
		if(!HTTPUtilities::isSuccess(curl_share_cleanup(curlSharedHandle))) {
			fmt::print("Failed to clean up CURL shared handle.");
		}
	});
}

HTTPUtilities::CURLStringList HTTPUtilities::createCURLStringList() {
	return CURLStringList(nullptr, [](curl_slist * curlStringList) {
		curl_slist_free_all(curlStringList);
	});
}

bool HTTPUtilities::appendToCURLStringList(CURLStringList & curlStringList, const std::string & string) {
	curl_slist * rawCURLStringList = curlStringList.get();
	curl_slist * rawUpdatedCURLStringList = nullptr;

	if((rawUpdatedCURLStringList = curl_slist_append(rawCURLStringList, string.c_str())) == nullptr) {
		return false;
	}

	curlStringList.release();
	curlStringList.reset(rawUpdatedCURLStringList);

	return true;
}

HTTPUtilities::CURLStringList HTTPUtilities::copyCURLStringList(const CURLStringList & curlStringList) {
	if(curlStringList == nullptr) {
		return nullptr;
	}

	CURLStringList newCURLStringList(createCURLStringList());
	const curl_slist * rawCURLStringListItem = curlStringList.get();

	while(rawCURLStringListItem != nullptr) {
		if(!appendToCURLStringList(newCURLStringList, rawCURLStringListItem->data)) {
			return nullptr;
		}

		rawCURLStringListItem = rawCURLStringListItem->next;
	}

	return newCURLStringList;
}

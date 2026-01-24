#include "HTTPUtilities.h"

#include "HTTPStatusCode.h"
#include "Utilities/StringUtilities.h"

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <optional>

std::string HTTPUtilities::getStatusCodeName(uint16_t statusCode) {
	std::optional<HTTPStatusCode> optionalStatusCode(magic_enum::enum_cast<HTTPStatusCode>(statusCode));

	if(!optionalStatusCode.has_value()) {
		return Utilities::emptyString;
	}

	return Utilities::toCapitalCase(magic_enum::enum_name(optionalStatusCode.value()));
}

std::string_view HTTPUtilities::getCURLErrorCodeName(CURLcode code) {
	switch(code) {
		case CURLE_OK:
			return "Ok";
		case CURLE_UNSUPPORTED_PROTOCOL:
			return "Unsupported Protocol";
		case CURLE_FAILED_INIT:
			return "Initialization Failed";
		case CURLE_URL_MALFORMAT:
			return "URL Malformatted";
		case CURLE_NOT_BUILT_IN:
			return "Not Built In";
		case CURLE_COULDNT_RESOLVE_PROXY:
			return "Couldn't Resolve Proxy";
		case CURLE_COULDNT_RESOLVE_HOST:
			return "Couldn't Resolve Host";
		case CURLE_COULDNT_CONNECT:
			return "Couldn't Connect";
		case CURLE_FTP_WEIRD_SERVER_REPLY:
			return "FTP Weird Server Reply";
		case CURLE_REMOTE_ACCESS_DENIED:
			return "Remote Access Denied";
		case CURLE_FTP_ACCEPT_FAILED:
			return "FTP Accept Failed";
		case CURLE_FTP_WEIRD_PASS_REPLY:
			return "FTP Weird Password Reply";
		case CURLE_FTP_ACCEPT_TIMEOUT:
			return "FTP Accept Timeout";
		case CURLE_FTP_WEIRD_PASV_REPLY:
			return "FTP Weird Passive Reply";
		case CURLE_FTP_WEIRD_227_FORMAT:
			return "FTP Weird 227 Format";
		case CURLE_FTP_CANT_GET_HOST:
			return "FTP Can't Get Host";
		case CURLE_OBSOLETE16:
			return "";
		case CURLE_FTP_COULDNT_SET_TYPE:
			return "FTP Couldn't Set Type";
		case CURLE_PARTIAL_FILE:
			return "Partial File";
		case CURLE_FTP_COULDNT_RETR_FILE:
			return "FTP Couldn't Retry File";
		case CURLE_OBSOLETE20:
			return "";
		case CURLE_QUOTE_ERROR:
			return "Quote Error";
		case CURLE_HTTP_RETURNED_ERROR:
			return "HTTP Returned Error";
		case CURLE_WRITE_ERROR:
			return "Write Error";
		case CURLE_OBSOLETE24:
			return "";
		case CURLE_UPLOAD_FAILED:
			return "Upload Failed";
		case CURLE_READ_ERROR:
			return "Read Error";
		case CURLE_OUT_OF_MEMORY:
			return "Out of Memory";
		case CURLE_OPERATION_TIMEDOUT:
			return "Operation Timedout";
		case CURLE_OBSOLETE29:
			return "";
		case CURLE_FTP_PORT_FAILED:
			return "FTP Port Failed";
		case CURLE_FTP_COULDNT_USE_REST:
			return "FTP Couldn't Use Rest";
		case CURLE_OBSOLETE32:
			return "";
		case CURLE_RANGE_ERROR:
			return "Range Error";
		case CURLE_HTTP_POST_ERROR:
			return "HTTP Post Error";
		case CURLE_SSL_CONNECT_ERROR:
			return "SSL Connect Error";
		case CURLE_BAD_DOWNLOAD_RESUME:
			return "Bad Download Resume";
		case CURLE_FILE_COULDNT_READ_FILE:
			return "File Couldn't Read File";
		case CURLE_LDAP_CANNOT_BIND:
			return "LDAP Cannot Bind";
		case CURLE_LDAP_SEARCH_FAILED:
			return "LDAP Search Failed";
		case CURLE_OBSOLETE40:
			return "";
		case CURLE_FUNCTION_NOT_FOUND:
			return "Function Not Found";
		case CURLE_ABORTED_BY_CALLBACK:
			return "Aborted by Callback";
		case CURLE_BAD_FUNCTION_ARGUMENT:
			return "Bad Function Argument";
		case CURLE_OBSOLETE44:
			return "";
		case CURLE_INTERFACE_FAILED:
			return "Interface Failed";
		case CURLE_OBSOLETE46:
			return "";
		case CURLE_TOO_MANY_REDIRECTS:
			return "Too Many Redirects";
		case CURLE_UNKNOWN_OPTION:
			return "Unknown Option";
		case CURLE_TELNET_OPTION_SYNTAX:
			return "Telnet Option Syntax";
		case CURLE_OBSOLETE50:
			return "";
		case CURLE_OBSOLETE51:
			return "";
		case CURLE_GOT_NOTHING:
			return "Got Nothing";
		case CURLE_SSL_ENGINE_NOTFOUND:
			return "SSL Engine Not Found";
		case CURLE_SSL_ENGINE_SETFAILED:
			return "SSL Engine Set Failed";
		case CURLE_SEND_ERROR:
			return "Send Error";
		case CURLE_RECV_ERROR:
			return "Receive Error";
		case CURLE_OBSOLETE57:
			return "";
		case CURLE_SSL_CERTPROBLEM:
			return "SSL Certificate Problem";
		case CURLE_SSL_CIPHER:
			return "SSL Cipher";
		case CURLE_PEER_FAILED_VERIFICATION:
			return "Peer Failed Verification";
		case CURLE_BAD_CONTENT_ENCODING:
			return "Bad Content Encoding";
		case CURLE_LDAP_INVALID_URL:
			return "LDAP Invalid URL";
		case CURLE_FILESIZE_EXCEEDED:
			return "File Size Exceeded";
		case CURLE_USE_SSL_FAILED:
			return "Use SSL Failed";
		case CURLE_SEND_FAIL_REWIND:
			return "Send Fail Rewind";
		case CURLE_SSL_ENGINE_INITFAILED:
			return "SSL Engine Initalization Failed";
		case CURLE_LOGIN_DENIED:
			return "Login Denied";
		case CURLE_TFTP_NOTFOUND:
			return "TFTP Not Found";
		case CURLE_TFTP_PERM:
			return "TFTP Permission Issue";
		case CURLE_REMOTE_DISK_FULL:
			return "Remote Disk Full";
		case CURLE_TFTP_ILLEGAL:
			return "TFTP Illegal";
		case CURLE_TFTP_UNKNOWNID:
			return "TFTP Unknown Identifier";
		case CURLE_REMOTE_FILE_EXISTS:
			return "Remote File Exists";
		case CURLE_TFTP_NOSUCHUSER:
			return "TFTP No Such User";
		case CURLE_OBSOLETE75:
			return "";
		case CURLE_OBSOLETE76:
			return "";
		case CURLE_SSL_CACERT_BADFILE:
			return "SSL CACert Bad File";
		case CURLE_REMOTE_FILE_NOT_FOUND:
			return "Remote File Not Found";
		case CURLE_SSH:
			return "SSH";
		case CURLE_SSL_SHUTDOWN_FAILED:
			return "SSL Shutdown Failed";
		case CURLE_AGAIN:
			return "Again";
		case CURLE_SSL_CRL_BADFILE:
			return "SSL CRL File Load Failed";
		case CURLE_SSL_ISSUER_ERROR:
			return "SSL Issuer Error";
		case CURLE_FTP_PRET_FAILED:
			return "FTP PRET Failed";
		case CURLE_RTSP_CSEQ_ERROR:
			return "RTSP CSequence Mismatch Error";
		case CURLE_RTSP_SESSION_ERROR:
			return "RTSP Session Identifier Mismatch Error";
		case CURLE_FTP_BAD_FILE_LIST:
			return "FTP Bad File List";
		case CURLE_CHUNK_FAILED:
			return "Chunk Failed";
		case CURLE_NO_CONNECTION_AVAILABLE:
			return "No Connection Available";
		case CURL_LAST:
			break;
	}

	return "";
}

std::string_view HTTPUtilities::getCURLErrorCodeName(CURLMcode code) {
	switch(code) {
		case CURLM_CALL_MULTI_PERFORM:
			return "Call Multi Perform";
		case CURLM_OK:
			return "Ok";
		case CURLM_BAD_HANDLE:
			return "Bad Handle";
		case CURLM_BAD_EASY_HANDLE:
			return "Bad Easy Handle";
		case CURLM_OUT_OF_MEMORY:
			return "Out of Memory";
		case CURLM_INTERNAL_ERROR:
			return "Internal Error";
		case CURLM_BAD_SOCKET:
			return "Bad Socket";
		case CURLM_UNKNOWN_OPTION:
			return "Unknown Option";
		case CURLM_ADDED_ALREADY:
			return "Added Already";
		case CURLM_RECURSIVE_API_CALL:
			return "Recursive API Call";
		case CURLM_WAKEUP_FAILURE:
			return "Wakeup Failure";
		case CURLM_BAD_FUNCTION_ARGUMENT:
			return "Bad Function Argument";
		case CURLM_ABORTED_BY_CALLBACK:
			return "Aborted by Callback";
		case CURLM_UNRECOVERABLE_POLL:
			return "Unrecoverable Poll";
	}

	return "";
}

std::string_view HTTPUtilities::getCURLErrorCodeName(CURLSHcode code) {
	switch(code) {
		case CURLSHE_OK:
			return "Ok";
		case CURLSHE_BAD_OPTION:
			return "Bad Option";
		case CURLSHE_IN_USE:
			return "In Use";
		case CURLSHE_INVALID:
			return "Invalid";
		case CURLSHE_NOMEM:
			return "No Memory";
		case CURLSHE_NOT_BUILT_IN:
			return "Not Built In";
	}

	return "";
}

bool HTTPUtilities::isSuccess(CURLcode code, const std::string & errorMessage) {
	if(code != CURLE_OK) {
		if(!errorMessage.empty()) {
			spdlog::error("{}: {}", errorMessage, getCURLErrorCodeName(code));
		}

		return false;
	}

	return true;
}

bool HTTPUtilities::isSuccess(CURLMcode code, const std::string & errorMessage) {
	if(code != CURLM_OK) {
		if(!errorMessage.empty()) {
			spdlog::error("{}: {}", errorMessage, getCURLErrorCodeName(code));
		}

		return false;
	}

	return true;
}

bool HTTPUtilities::isSuccess(CURLSHcode code, const std::string & errorMessage) {
	if(code != CURLSHE_OK) {
		if(!errorMessage.empty()) {
			spdlog::error("{}: {}", errorMessage, getCURLErrorCodeName(code));
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
			spdlog::error("Failed to clean up CURL multi handle.");
		}
	});
}

HTTPUtilities::CURLSharedHandle HTTPUtilities::createCURLSharedHandle() {
	return CURLSharedHandle(curl_share_init(), [](CURLSH * curlSharedHandle) {
		if(!HTTPUtilities::isSuccess(curl_share_cleanup(curlSharedHandle))) {
			spdlog::error("Failed to clean up CURL shared handle.");
		}
	});
}

std::string HTTPUtilities::easyEscape(CURL * handle, std::string_view url) {
	if(handle == nullptr || url.empty()) {
		return {};
	}

	std::unique_ptr<char, std::function<void (char *)>> escapedURL(curl_easy_escape(handle, url.data(), url.length()), [](char * escapedURL) {
		curl_free(escapedURL);
	});

	return std::string(escapedURL.get());
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

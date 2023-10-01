#include "TimeZoneDataManagerWindows.h"

#include "Network/HTTPService.h"
#include "Platform/DeviceInformationBridge.h"
#include "Utilities/FileUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

static const std::string WINDOWS_TIMEZONE_DATA_FILE_NAME("windowsZones.xml");
static const std::string WINDOWS_TIMEZONE_DATA_DOWNLOAD_URL(Utilities::joinPaths("https://raw.githubusercontent.com/unicode-org/cldr/master/common/supplemental", WINDOWS_TIMEZONE_DATA_FILE_NAME));

TimeZoneDataManagerWindows::TimeZoneDataManagerWindows() { }

TimeZoneDataManagerWindows::~TimeZoneDataManagerWindows() { }

bool TimeZoneDataManagerWindows::platformInitialize(const std::string & dataDirectoryPath, std::map<std::string, std::string> & fileETags, bool shouldUpdate, bool forceUpdate, bool * updated) {
	std::string windowsTimeZoneFilePath(Utilities::joinPaths(dataDirectoryPath, WINDOWS_TIMEZONE_DATA_FILE_NAME));
	bool windowsTimeZoneDataFileExists = std::filesystem::is_regular_file(std::filesystem::path(windowsTimeZoneFilePath));

	if(windowsTimeZoneDataFileExists && !shouldUpdate && !forceUpdate) {
		return true;
	}

	if(!DeviceInformationBridge::getInstance()->isConnectedToInternet()) {
		return windowsTimeZoneDataFileExists;
	}

	std::string windowsTimeZoneFileETag;

	std::map<std::string, std::string>::const_iterator windowsTimeZoneFileETagIterator = fileETags.find(WINDOWS_TIMEZONE_DATA_FILE_NAME);

	if(windowsTimeZoneFileETagIterator != fileETags.end()) {
		windowsTimeZoneFileETag = windowsTimeZoneFileETagIterator->second;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::warn("Attempted to initialize Windows time zone data manager without initializing HTTP service first!");

		return windowsTimeZoneDataFileExists;
	}

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, WINDOWS_TIMEZONE_DATA_DOWNLOAD_URL));

	if(!forceUpdate && windowsTimeZoneDataFileExists && !windowsTimeZoneFileETag.empty()) {
		request->setIfNoneMatchETag(windowsTimeZoneFileETag);
	}

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response == nullptr || response->isFailure()) {
		spdlog::error("Failed to download Windows time zone data file with error: {}", response != nullptr ? response->getErrorMessage() : "Invalid request.");
		return windowsTimeZoneDataFileExists;
	}
	else if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::debug("Windows time zone data file is already up to date!");

		if(updated != nullptr) {
			*updated = true;
		}

		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download Windows time zone data file ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return windowsTimeZoneDataFileExists;
	}

	spdlog::debug("Windows time zone data file downloaded successfully after {} ms.", response->getRequestDuration().value().count());

	if(!response->getBody()->writeTo(windowsTimeZoneFilePath, true)) {
		spdlog::error("Failed to write Windows time zone data file to: '{}'.", windowsTimeZoneFilePath);
		return windowsTimeZoneDataFileExists;
	}

	if(updated != nullptr) {
		*updated = true;
	}

	fileETags.emplace(WINDOWS_TIMEZONE_DATA_FILE_NAME, response->getETag());

	spdlog::info("Latest Windows time zone data file written to: '{}'.", windowsTimeZoneFilePath);

	return true;
}

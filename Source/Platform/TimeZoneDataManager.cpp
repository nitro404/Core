#include "TimeZoneDataManager.h"

#include "Archive/ArchiveFactoryRegistry.h"
#include "Network/HTTPService.h"
#include "Platform/DeviceInformationBridge.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <date/tz.h>
#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <fstream>

static const std::string IANA_TIME_ZONE_DATABASE_PAGE_URL("https://www.iana.org/time-zones");
static const std::string IANA_DATA_BASE_URL("https://data.iana.org");
static const std::string IANA_TIME_ZONE_DATABASE_RELEASES_SUFFIX("time-zones/releases/tzdata");
static const std::string TIME_ZONE_DATABASE_ARCHIVE_FILE_EXTENSION("tar.gz");
static const std::string IANA_TIME_ZONE_DATABASE_RELEASES_BASE_URL(Utilities::joinPaths(IANA_DATA_BASE_URL, IANA_TIME_ZONE_DATABASE_RELEASES_SUFFIX));

static const std::array<std::string, 17> IANA_TIME_ZONE_DATABASE_FILE_NAMES = {
	"africa",
	"antarctica",
	"asia",
	"australasia",
	"backward",
	"backzone",
	"calendars",
	"etcetera",
	"europe",
	"factory",
	"iso3166.tab",
	"leapseconds",
	"leap-seconds.list",
	"northamerica",
	"southamerica",
	"zone.tab",
	"zone1970.tab"
};

TimeZoneDataManager::TimeZoneDataManager()
	: m_initialized(false) { }

TimeZoneDataManager::~TimeZoneDataManager() { }

bool TimeZoneDataManager::isSupported() const {
	return false;
}

bool TimeZoneDataManager::isInitialized() const {
	return m_initialized;
}

bool TimeZoneDataManager::initialize(const std::string & dataDirectoryPath, std::map<std::string, std::string> & fileETags, bool shouldUpdate, bool forceUpdate, bool * updated) {
	if(!isSupported()) {
		spdlog::warn("Time zone data manager is not supported on this platform.");
		return false;
	}

	if(m_initialized) {
		return true;
	}

	if(!platformInitialize(dataDirectoryPath, fileETags, shouldUpdate, forceUpdate, updated)) {
		return false;
	}

	if(!updateTimeZoneDatabase(dataDirectoryPath, fileETags, shouldUpdate, forceUpdate, updated)) {
		return false;
	}

	date::set_install(dataDirectoryPath);

	m_initialized = true;

	return true;
}

bool TimeZoneDataManager::platformInitialize(const std::string & dataDirectoryPath, std::map<std::string, std::string> & fileETags, bool shouldUpdate, bool forceUpdate, bool * updated) {
	return true;
}

std::string TimeZoneDataManager::getCurrentTimeZoneDatabaseVersion(const std::string & dataDirectoryPath) {
	static const std::string TIME_ZONE_DATABASE_VERSION_FILE_NAME("version");

	std::string timeZoneDatabaseFilePath(Utilities::joinPaths(dataDirectoryPath, TIME_ZONE_DATABASE_VERSION_FILE_NAME));

	if(!std::filesystem::is_regular_file(std::filesystem::path(timeZoneDatabaseFilePath))) {
		return {};
	}

	std::ifstream fileStream(timeZoneDatabaseFilePath);

	if(!fileStream.is_open()) {
		return {};
	}

	std::string currentTimeZoneDatabaseVersion;

	while(std::getline(fileStream, currentTimeZoneDatabaseVersion)) {
		currentTimeZoneDatabaseVersion = Utilities::trimString(currentTimeZoneDatabaseVersion);

		if(!currentTimeZoneDatabaseVersion.empty()) {
			break;
		}
	}

	return currentTimeZoneDatabaseVersion;
}

bool TimeZoneDataManager::doAllTimeZoneDatabaseFilesExist(const std::string & dataDirectoryPath) {
	for(const std::string & fileName : IANA_TIME_ZONE_DATABASE_FILE_NAMES) {
		if(!std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(dataDirectoryPath, fileName)))) {
			return false;
		}
	}

	return true;
}

std::string TimeZoneDataManager::getLatestTimeZoneDatabaseVersion() {
	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::warn("Attempted to retrieve latest time zone database download URL without initializing HTTP service first!");
		return {};
	}

	std::shared_ptr<HTTPRequest> timeZoneDatabasePageRequest(httpService->createRequest(HTTPRequest::Method::Get, IANA_TIME_ZONE_DATABASE_PAGE_URL));

	std::shared_ptr<HTTPResponse> timeZoneDatabasePageResponse(httpService->sendRequestAndWait(timeZoneDatabasePageRequest));

	if(timeZoneDatabasePageResponse == nullptr || timeZoneDatabasePageResponse->isFailure()) {
		spdlog::error("Failed to retrieve Internet Assigned Numbers Authority time zone database page with error: {}", timeZoneDatabasePageResponse != nullptr ? timeZoneDatabasePageResponse->getErrorMessage() : "Invalid request.");
		return {};
	}
	else if(timeZoneDatabasePageResponse->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(timeZoneDatabasePageResponse->getStatusCode()));
		spdlog::error("Failed to retrieve Internet Assigned Numbers Authority time zone database page ({}{})!", timeZoneDatabasePageResponse->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	spdlog::debug("Retrieved Internet Assigned Numbers Authority time zone database page after {} ms.", timeZoneDatabasePageResponse->getRequestDuration().value().count());

	std::string timeZoneDatabasePageHTML(timeZoneDatabasePageResponse->getBodyAsString());

	timeZoneDatabasePageRequest.reset();
	timeZoneDatabasePageResponse.reset();

	size_t latestTimeZoneDatabaseFileNameIndex = timeZoneDatabasePageHTML.find(IANA_TIME_ZONE_DATABASE_RELEASES_SUFFIX);

	if(latestTimeZoneDatabaseFileNameIndex == std::string::npos) {
		spdlog::error("Failed to determine latest time zone database file name from page HTML data.");
		return {};
	}

	size_t latestTimeZoneDatabaseVersionStartIndex = latestTimeZoneDatabaseFileNameIndex + IANA_TIME_ZONE_DATABASE_RELEASES_SUFFIX.length();

	size_t latestTimeZoneDataBaseFileExtensionIndex = timeZoneDatabasePageHTML.find(TIME_ZONE_DATABASE_ARCHIVE_FILE_EXTENSION, latestTimeZoneDatabaseFileNameIndex);

	if(latestTimeZoneDataBaseFileExtensionIndex == std::string::npos) {
		spdlog::error("Failed to find latest time zone database file extension in page HTML data.");
		return {};
	}

	return std::string(timeZoneDatabasePageHTML.c_str() + latestTimeZoneDatabaseVersionStartIndex, latestTimeZoneDataBaseFileExtensionIndex - latestTimeZoneDatabaseVersionStartIndex - 1);
}

std::string TimeZoneDataManager::getTimeZoneDatabaseDownloadURL(const std::string & timeZoneDatabaseVersion) {
	if(timeZoneDatabaseVersion.empty()) {
		return {};
	}

	return IANA_TIME_ZONE_DATABASE_RELEASES_BASE_URL + timeZoneDatabaseVersion + "." + TIME_ZONE_DATABASE_ARCHIVE_FILE_EXTENSION;
}

std::string TimeZoneDataManager::getLatestTimeZoneDatabaseDownloadURL() {
	return getTimeZoneDatabaseDownloadURL(getLatestTimeZoneDatabaseVersion());
}

bool TimeZoneDataManager::updateTimeZoneDatabase(const std::string & dataDirectoryPath, std::map<std::string, std::string> & fileETags, bool shouldUpdate, bool forceUpdate, bool * updated) {
	bool allTimeZoneDatabaseFilesExist = doAllTimeZoneDatabaseFilesExist(dataDirectoryPath);

	if(allTimeZoneDatabaseFilesExist && !shouldUpdate && !forceUpdate) {
		return true;
	}

	if(!DeviceInformationBridge::getInstance()->isConnectedToInternet()) {
		return allTimeZoneDatabaseFilesExist;
	}

	std::string latestTimeZoneDatabaseVersion(getLatestTimeZoneDatabaseVersion());

	if(latestTimeZoneDatabaseVersion.empty()) {
		return allTimeZoneDatabaseFilesExist;
	}

	if(Utilities::areStringsEqualIgnoreCase(getCurrentTimeZoneDatabaseVersion(dataDirectoryPath), latestTimeZoneDatabaseVersion)) {
		spdlog::debug("Latest '{}' Internet Assigned Numbers Authority time zone database files are already installed!", latestTimeZoneDatabaseVersion);

		if(updated != nullptr) {
			*updated = true;
		}

		return true;
	}

	std::string latestTimeZoneDatabaseDownloadURL(getTimeZoneDatabaseDownloadURL(latestTimeZoneDatabaseVersion));
	std::string latestTimeZoneDatabaseArchiveFileName(Utilities::getFileName(latestTimeZoneDatabaseDownloadURL));
	std::string timeZoneDatabaseArchiveETag;
	std::map<std::string, std::string>::const_iterator timeZoneDatabaseArchiveETagIterator = fileETags.find(latestTimeZoneDatabaseArchiveFileName);

	if(timeZoneDatabaseArchiveETagIterator != fileETags.end()) {
		timeZoneDatabaseArchiveETag = timeZoneDatabaseArchiveETagIterator->second;
	}

	HTTPService * httpService = HTTPService::getInstance();

	std::shared_ptr<HTTPRequest> latestTimeZoneDatabaseRequest(httpService->createRequest(HTTPRequest::Method::Get, latestTimeZoneDatabaseDownloadURL));

	if(!forceUpdate && allTimeZoneDatabaseFilesExist && !timeZoneDatabaseArchiveETag.empty()) {
		latestTimeZoneDatabaseRequest->setIfNoneMatchETag(timeZoneDatabaseArchiveETag);
	}

	std::shared_ptr<HTTPResponse> latestTimeZoneDatabaseResponse(httpService->sendRequestAndWait(latestTimeZoneDatabaseRequest));

	if(latestTimeZoneDatabaseResponse == nullptr || latestTimeZoneDatabaseResponse->isFailure()) {
		spdlog::error("Failed to download latest Internet Assigned Numbers Authority time zone database archive file with error: {}", latestTimeZoneDatabaseResponse != nullptr ? latestTimeZoneDatabaseResponse->getErrorMessage() : "Invalid request.");
		return allTimeZoneDatabaseFilesExist;
	}
	else if(latestTimeZoneDatabaseResponse->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::debug("Internet Assigned Numbers Authority time zone database is already up to date with version '{}'!", latestTimeZoneDatabaseVersion);

		if(updated != nullptr) {
			*updated = true;
		}

		return true;
	}
	else if(latestTimeZoneDatabaseResponse->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(latestTimeZoneDatabaseResponse->getStatusCode()));
		spdlog::error("Failed to download latest Internet Assigned Numbers Authority time zone database archive file ({}{})!", latestTimeZoneDatabaseResponse->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return allTimeZoneDatabaseFilesExist;
	}

	spdlog::debug("Latest Internet Assigned Numbers Authority time zone database archive file downloaded successfully after {} ms.", latestTimeZoneDatabaseResponse->getRequestDuration().value().count());

	std::unique_ptr<Archive> latestTimeZoneDatabaseArchive(ArchiveFactoryRegistry::getInstance()->createArchiveFrom(latestTimeZoneDatabaseResponse->transferBody(), latestTimeZoneDatabaseDownloadURL));

	if(latestTimeZoneDatabaseArchive == nullptr) {
		spdlog::error("Failed to create archive handle from '{}' time zone database archive file!", latestTimeZoneDatabaseArchiveFileName);
		return allTimeZoneDatabaseFilesExist;
	}

	if(latestTimeZoneDatabaseArchive->extractAllEntries(dataDirectoryPath, true) == 0) {
		spdlog::error("Failed to extract Internet Assigned Numbers Authority time zone database archive '{}' to directory: '{}'!", latestTimeZoneDatabaseArchiveFileName, dataDirectoryPath);
		return allTimeZoneDatabaseFilesExist;
	}

	if(updated != nullptr) {
		*updated = true;
	}

	fileETags.emplace(latestTimeZoneDatabaseArchiveFileName, latestTimeZoneDatabaseResponse->getETag());

	return true;
}

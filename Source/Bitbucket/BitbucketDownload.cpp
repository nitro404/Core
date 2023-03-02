#include "BitbucketDownload.h"

#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <spdlog/spdlog.h>

static constexpr const char * JSON_FILE_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_FILE_SIZE_PROPERTY_NAME = "size";
static constexpr const char * JSON_CREATED_TIMESTAMP_PROPERTY_NAME = "created_on";
static constexpr const char * JSON_DOWNLOAD_LINKS_PROPERTY_NAME = "links";
static constexpr const char * JSON_DOWNLOAD_LINKS_SELF_PROPERTY_NAME = "self";
static constexpr const char * JSON_DOWNLOAD_LINKS_SELF_HREF_PROPERTY_NAME = "href";

BitbucketDownload::BitbucketDownload(const std::string & fileName, uint64_t fileSize, const std::string & downloadURL, std::chrono::time_point<std::chrono::system_clock> createdTimestamp)
	: m_fileName(fileName)
	, m_fileSize(fileSize)
	, m_downloadURL(downloadURL)
	, m_createdTimestamp(createdTimestamp) { }

BitbucketDownload::BitbucketDownload(BitbucketDownload && a) noexcept
	: m_fileName(std::move(a.m_fileName))
	, m_fileSize(a.m_fileSize)
	, m_downloadURL(std::move(a.m_downloadURL))
	, m_createdTimestamp(a.m_createdTimestamp) { }

BitbucketDownload::BitbucketDownload(const BitbucketDownload & a)
	: m_fileName(a.m_fileName)
	, m_fileSize(a.m_fileSize)
	, m_downloadURL(a.m_downloadURL)
	, m_createdTimestamp(a.m_createdTimestamp) { }

BitbucketDownload & BitbucketDownload::operator = (BitbucketDownload && a) noexcept {
	if(this != &a) {
		m_fileName = std::move(a.m_fileName);
		m_fileSize = a.m_fileSize;
		m_downloadURL = std::move(a.m_downloadURL);
		m_createdTimestamp = a.m_createdTimestamp;
	}

	return *this;
}

BitbucketDownload & BitbucketDownload::operator = (const BitbucketDownload & a) {
	m_fileName = a.m_fileName;
	m_fileSize = a.m_fileSize;
	m_downloadURL = a.m_downloadURL;
	m_createdTimestamp = a.m_createdTimestamp;

	return *this;
}

BitbucketDownload::~BitbucketDownload() = default;

const std::string & BitbucketDownload::getFileName() const {
	return m_fileName;
}

uint64_t BitbucketDownload::getFileSize() const {
	return m_fileSize;
}

const std::string & BitbucketDownload::getDownloadURL() const {
	return m_downloadURL;
}

std::chrono::time_point<std::chrono::system_clock> BitbucketDownload::getCreatedTimestamp() const {
	return m_createdTimestamp;
}

std::unique_ptr<BitbucketDownload> BitbucketDownload::parseFrom(const rapidjson::Value & downloadValue) {
	if(!downloadValue.IsObject()) {
		spdlog::error("Invalid Bitbucket download type: '{}', expected 'object'.", Utilities::typeToString(downloadValue.GetType()));
		return nullptr;
	}

	// parse file name
	if(!downloadValue.HasMember(JSON_FILE_NAME_PROPERTY_NAME)) {
		spdlog::error("Bitbucket download is missing '{}' property'.", JSON_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileNameValue = downloadValue[JSON_FILE_NAME_PROPERTY_NAME];

	if(!fileNameValue.IsString()) {
		spdlog::error("Bitbucket download has an invalid '{}' property type: '{}', expected 'string'.", JSON_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(fileNameValue.GetType()));
		return nullptr;
	}

	std::string fileName(Utilities::trimString(fileNameValue.GetString()));

	if(fileName.empty()) {
		spdlog::error("Bitbucket download '{}' property cannot be empty.", JSON_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse file size
	if(!downloadValue.HasMember(JSON_FILE_SIZE_PROPERTY_NAME)) {
		spdlog::error("Bitbucket download is missing '{}' property'.", JSON_FILE_SIZE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileSizeValue = downloadValue[JSON_FILE_SIZE_PROPERTY_NAME];

	if(!fileSizeValue.IsUint64()) {
		spdlog::error("Bitbucket download has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_FILE_SIZE_PROPERTY_NAME, Utilities::typeToString(fileSizeValue.GetType()));
		return nullptr;
	}

	uint64_t fileSize = fileSizeValue.GetUint64();

	// parse created timestamp
	if(!downloadValue.HasMember(JSON_CREATED_TIMESTAMP_PROPERTY_NAME)) {
		spdlog::error("Bitbucket download is missing '{}' property'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & createdTimestampValue = downloadValue[JSON_CREATED_TIMESTAMP_PROPERTY_NAME];

	if(!createdTimestampValue.IsString()) {
		spdlog::error("Bitbucket download has an invalid '{}' property type: '{}', expected 'string'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(createdTimestampValue.GetType()));
		return nullptr;
	}

	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalCreatedTimestamp(Utilities::parseTimePointFromString(createdTimestampValue.GetString()));

	if(!optionalCreatedTimestamp.has_value()) {
		spdlog::error("Bitbucket download has an invalid '{}' property value: '{}'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME, createdTimestampValue.GetString());
		return nullptr;
	}

	// get links object
	if(!downloadValue.HasMember(JSON_DOWNLOAD_LINKS_PROPERTY_NAME)) {
		spdlog::error("Bitbucket download is missing '{}' property'.", JSON_DOWNLOAD_LINKS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & linksValue = downloadValue[JSON_DOWNLOAD_LINKS_PROPERTY_NAME];

	if(!linksValue.IsObject()) {
		spdlog::error("Bitbucket download has an invalid '{}' property type: '{}', expected 'object'.", JSON_DOWNLOAD_LINKS_PROPERTY_NAME, Utilities::typeToString(linksValue.GetType()));
		return nullptr;
	}

	// get links self object
	if(!linksValue.HasMember(JSON_DOWNLOAD_LINKS_SELF_PROPERTY_NAME)) {
		spdlog::error("Bitbucket download is missing '{}' property'.", JSON_DOWNLOAD_LINKS_SELF_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & linksSelfValue = linksValue[JSON_DOWNLOAD_LINKS_SELF_PROPERTY_NAME];

	if(!linksSelfValue.IsObject()) {
		spdlog::error("Bitbucket download has an invalid '{}' '{}' property type: '{}', expected 'object'.", JSON_DOWNLOAD_LINKS_PROPERTY_NAME, JSON_DOWNLOAD_LINKS_SELF_PROPERTY_NAME, Utilities::typeToString(linksSelfValue.GetType()));
		return nullptr;
	}

	// parse links self href value (download url)
	if(!linksSelfValue.HasMember(JSON_DOWNLOAD_LINKS_SELF_HREF_PROPERTY_NAME)) {
		spdlog::error("Bitbucket download is missing '{}' '{}' '{}' property'.", JSON_DOWNLOAD_LINKS_PROPERTY_NAME, JSON_DOWNLOAD_LINKS_SELF_PROPERTY_NAME, JSON_DOWNLOAD_LINKS_SELF_HREF_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & linksSelfHrefValue = linksSelfValue[JSON_DOWNLOAD_LINKS_SELF_HREF_PROPERTY_NAME];

	if(!linksSelfHrefValue.IsString()) {
		spdlog::error("Bitbucket download has an invalid '{}' '{}' '{}' property type: '{}', expected 'string'.", JSON_DOWNLOAD_LINKS_PROPERTY_NAME, JSON_DOWNLOAD_LINKS_SELF_PROPERTY_NAME, JSON_DOWNLOAD_LINKS_SELF_HREF_PROPERTY_NAME, Utilities::typeToString(linksSelfHrefValue.GetType()));
		return nullptr;
	}

	std::string downloadURL(Utilities::trimString(linksSelfHrefValue.GetString()));

	if(downloadURL.empty()) {
		spdlog::error("Bitbucket download '{}' '{}' '{}' property cannot be empty.", JSON_DOWNLOAD_LINKS_PROPERTY_NAME, JSON_DOWNLOAD_LINKS_SELF_PROPERTY_NAME, JSON_DOWNLOAD_LINKS_SELF_HREF_PROPERTY_NAME);
		return nullptr;
	}

	return std::make_unique<BitbucketDownload>(fileName, fileSize, downloadURL, optionalCreatedTimestamp.value());
}

bool BitbucketDownload::isValid() const {
	return !m_fileName.empty() &&
		   !m_downloadURL.empty();
}

bool BitbucketDownload::isValid(const BitbucketDownload * a) {
		return a != nullptr && a->isValid();
}

bool BitbucketDownload::operator == (const BitbucketDownload & a) const {
	return m_fileSize == a.m_fileSize &&
		   m_createdTimestamp == a.m_createdTimestamp &&
		   Utilities::areStringsEqual(m_fileName, a.m_fileName) &&
		   Utilities::areStringsEqual(m_downloadURL, a.m_downloadURL);
}

bool BitbucketDownload::operator != (const BitbucketDownload & a) const {
	return !operator == (a);
}

#include "GitHubReleaseAsset.h"

#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <spdlog/spdlog.h>

static constexpr const char * JSON_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_FILE_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_FILE_SIZE_PROPERTY_NAME = "size";
static constexpr const char * JSON_LABEL_PROPERTY_NAME = "label";
static constexpr const char * JSON_ASSET_URL_PROPERTY_NAME = "url";
static constexpr const char * JSON_DOWNLOAD_URL_PROPERTY_NAME = "browser_download_url";
static constexpr const char * JSON_CONTENT_TYPE_PROPERTY_NAME = "content_type";
static constexpr const char * JSON_CREATED_TIMESTAMP_PROPERTY_NAME = "created_at";
static constexpr const char * JSON_UPDATED_TIMESTAMP_PROPERTY_NAME = "updated_at";

GitHubReleaseAsset::GitHubReleaseAsset(uint64_t id, const std::string & fileName, uint64_t fileSize, const std::string & label,  const std::string & assetURL, const std::string & downloadURL, const std::string & contentType, std::chrono::time_point<std::chrono::system_clock> createdTimestamp, std::chrono::time_point<std::chrono::system_clock> updatedTimestamp)
	: m_id(id)
	, m_fileName(fileName)
	, m_fileSize(fileSize)
	, m_label(label)
	, m_assetURL(assetURL)
	, m_downloadURL(downloadURL)
	, m_contentType(contentType)
	, m_createdTimestamp(createdTimestamp)
	, m_updatedTimestamp(updatedTimestamp)
	, m_parentGitHubRelease(nullptr) { }

GitHubReleaseAsset::GitHubReleaseAsset(GitHubReleaseAsset && a) noexcept
	: m_id(a.m_id)
	, m_fileName(std::move(a.m_fileName))
	, m_fileSize(a.m_fileSize)
	, m_label(std::move(a.m_label))
	, m_assetURL(std::move(a.m_assetURL))
	, m_downloadURL(std::move(a.m_downloadURL))
	, m_contentType(std::move(a.m_contentType))
	, m_createdTimestamp(a.m_createdTimestamp)
	, m_updatedTimestamp(a.m_updatedTimestamp)
	, m_parentGitHubRelease(nullptr) { }

GitHubReleaseAsset::GitHubReleaseAsset(const GitHubReleaseAsset & a)
	: m_id(a.m_id)
	, m_fileName(a.m_fileName)
	, m_fileSize(a.m_fileSize)
	, m_label(a.m_label)
	, m_assetURL(a.m_assetURL)
	, m_downloadURL(a.m_downloadURL)
	, m_contentType(a.m_contentType)
	, m_createdTimestamp(a.m_createdTimestamp)
	, m_updatedTimestamp(a.m_updatedTimestamp)
	, m_parentGitHubRelease(nullptr) { }

GitHubReleaseAsset & GitHubReleaseAsset::operator = (GitHubReleaseAsset && a) noexcept {
	if(this != &a) {
		m_id = a.m_id;
		m_fileName = std::move(a.m_fileName);
		m_fileSize = a.m_fileSize;
		m_label = std::move(a.m_label);
		m_assetURL = std::move(a.m_assetURL);
		m_downloadURL = std::move(a.m_downloadURL);
		m_contentType = std::move(a.m_contentType);
		m_createdTimestamp = a.m_createdTimestamp;
		m_updatedTimestamp = a.m_updatedTimestamp;
	}

	return *this;
}

GitHubReleaseAsset & GitHubReleaseAsset::operator = (const GitHubReleaseAsset & a) {
	m_id = a.m_id;
	m_fileName = a.m_fileName;
	m_fileSize = a.m_fileSize;
	m_label = a.m_label;
	m_assetURL = a.m_assetURL;
	m_downloadURL = a.m_downloadURL;
	m_contentType = a.m_contentType;
	m_createdTimestamp = a.m_createdTimestamp;
	m_updatedTimestamp = a.m_updatedTimestamp;

	return *this;
}

GitHubReleaseAsset::~GitHubReleaseAsset() {
	m_parentGitHubRelease = nullptr;
}

uint64_t GitHubReleaseAsset::getID() const {
	return m_id;
}

const std::string & GitHubReleaseAsset::getFileName() const {
	return m_fileName;
}

uint64_t GitHubReleaseAsset::getFileSize() const {
	return m_fileSize;
}

const std::string & GitHubReleaseAsset::getAssetURL() const {
	return m_assetURL;
}

const std::string & GitHubReleaseAsset::getDownloadURL() const {
	return m_downloadURL;
}

const std::string & GitHubReleaseAsset::getLabel() const {
	return m_label;
}

const std::string & GitHubReleaseAsset::getContentType() const {
	return m_contentType;
}

std::chrono::time_point<std::chrono::system_clock> GitHubReleaseAsset::getCreatedTimestamp() const {
	return m_createdTimestamp;
}

std::chrono::time_point<std::chrono::system_clock> GitHubReleaseAsset::getUpdatedTimestamp() const {
	return m_updatedTimestamp;
}

const GitHubRelease * GitHubReleaseAsset::getParentGitHubRelease() const {
	return m_parentGitHubRelease;
}

void GitHubReleaseAsset::setParentGitHubRelease(const GitHubRelease * release) {
	m_parentGitHubRelease = release;
}

std::unique_ptr<GitHubReleaseAsset> GitHubReleaseAsset::parseFrom(const rapidjson::Value & assetValue) {
	if(!assetValue.IsObject()) {
		spdlog::error("Invalid GitHub release asset type: '{}', expected 'object'.", Utilities::typeToString(assetValue.GetType()));
		return nullptr;
	}

	// parse id
	if(!assetValue.HasMember(JSON_ID_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & idValue = assetValue[JSON_ID_PROPERTY_NAME];

	if(!idValue.IsUint64()) {
		spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_ID_PROPERTY_NAME, Utilities::typeToString(idValue.GetType()));
		return nullptr;
	}

	uint64_t id = idValue.GetUint64();

	// parse file name
	if(!assetValue.HasMember(JSON_FILE_NAME_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileNameValue = assetValue[JSON_FILE_NAME_PROPERTY_NAME];

	if(!fileNameValue.IsString()) {
		spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected 'string'.", JSON_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(fileNameValue.GetType()));
		return nullptr;
	}

	std::string fileName(Utilities::trimString(fileNameValue.GetString()));

	if(fileName.empty()) {
		spdlog::error("GitHub release asset '{}' property cannot be empty.", JSON_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse file size
	if(!assetValue.HasMember(JSON_FILE_SIZE_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_FILE_SIZE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileSizeValue = assetValue[JSON_FILE_SIZE_PROPERTY_NAME];

	if(!fileSizeValue.IsUint64()) {
		spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_FILE_SIZE_PROPERTY_NAME, Utilities::typeToString(fileSizeValue.GetType()));
		return nullptr;
	}

	uint64_t fileSize = fileSizeValue.GetUint64();

	// parse label
	if(!assetValue.HasMember(JSON_LABEL_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_LABEL_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & labelValue = assetValue[JSON_LABEL_PROPERTY_NAME];

	std::string label;

	if(!labelValue.IsNull()) {
		if(!labelValue.IsString()) {
			spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected 'string'.", JSON_LABEL_PROPERTY_NAME, Utilities::typeToString(labelValue.GetType()));
			return nullptr;
		}

		label = Utilities::trimString(labelValue.GetString());
	}

	// parse asset url
	if(!assetValue.HasMember(JSON_ASSET_URL_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_ASSET_URL_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & assetURLValue = assetValue[JSON_ASSET_URL_PROPERTY_NAME];

	if(!assetURLValue.IsString()) {
		spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected 'string'.", JSON_ASSET_URL_PROPERTY_NAME, Utilities::typeToString(assetURLValue.GetType()));
		return nullptr;
	}

	std::string assetURL(Utilities::trimString(assetURLValue.GetString()));

	if(assetURL.empty()) {
		spdlog::error("GitHub release asset '{}' property cannot be empty.", JSON_ASSET_URL_PROPERTY_NAME);
		return nullptr;
	}

	// parse download url
	if(!assetValue.HasMember(JSON_DOWNLOAD_URL_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_DOWNLOAD_URL_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & downloadUrlValue = assetValue[JSON_DOWNLOAD_URL_PROPERTY_NAME];

	if(!downloadUrlValue.IsString()) {
		spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected 'string'.", JSON_DOWNLOAD_URL_PROPERTY_NAME, Utilities::typeToString(downloadUrlValue.GetType()));
		return nullptr;
	}

	std::string downloadURL(Utilities::trimString(downloadUrlValue.GetString()));

	if(downloadURL.empty()) {
		spdlog::error("GitHub release asset '{}' property cannot be empty.", JSON_DOWNLOAD_URL_PROPERTY_NAME);
		return nullptr;
	}

	// parse content type
	if(!assetValue.HasMember(JSON_CONTENT_TYPE_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_CONTENT_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & contentTypeValue = assetValue[JSON_CONTENT_TYPE_PROPERTY_NAME];

	if(!contentTypeValue.IsString()) {
		spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected 'string'.", JSON_CONTENT_TYPE_PROPERTY_NAME, Utilities::typeToString(contentTypeValue.GetType()));
		return nullptr;
	}

	std::string contentType(Utilities::trimString(contentTypeValue.GetString()));

	if(contentType.empty()) {
		spdlog::error("GitHub release asset '{}' property cannot be empty.", JSON_CONTENT_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	// parse created timestamp
	if(!assetValue.HasMember(JSON_CREATED_TIMESTAMP_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & createdTimestampValue = assetValue[JSON_CREATED_TIMESTAMP_PROPERTY_NAME];

	if(!createdTimestampValue.IsString()) {
		spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected 'string'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(createdTimestampValue.GetType()));
		return nullptr;
	}

	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalCreatedTimestamp(Utilities::parseTimePointFromString(createdTimestampValue.GetString()));

	if(!optionalCreatedTimestamp.has_value()) {
		spdlog::error("GitHub release asset has an invalid '{}' property value: '{}'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME, createdTimestampValue.GetString());
		return nullptr;
	}

	// parse updated timestamp
	if(!assetValue.HasMember(JSON_UPDATED_TIMESTAMP_PROPERTY_NAME)) {
		spdlog::error("GitHub release asset is missing '{}' property.", JSON_UPDATED_TIMESTAMP_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & updatedTimestampValue = assetValue[JSON_UPDATED_TIMESTAMP_PROPERTY_NAME];

	if(!updatedTimestampValue.IsString()) {
		spdlog::error("GitHub release asset has an invalid '{}' property type: '{}', expected 'string'.", JSON_UPDATED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(updatedTimestampValue.GetType()));
		return nullptr;
	}

	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalUpdatedTimestamp(Utilities::parseTimePointFromString(updatedTimestampValue.GetString()));

	if(!optionalUpdatedTimestamp.has_value()) {
		spdlog::error("GitHub release asset has an invalid '{}' property value: '{}'.", JSON_UPDATED_TIMESTAMP_PROPERTY_NAME, updatedTimestampValue.GetString());
		return nullptr;
	}

	return std::make_unique<GitHubReleaseAsset>(id, fileName, fileSize, label, assetURL, downloadURL, contentType, optionalCreatedTimestamp.value(), optionalUpdatedTimestamp.value());
}

bool GitHubReleaseAsset::isValid() const {
	return m_id != 0 &&
		   !m_fileName.empty() &&
		   !m_assetURL.empty() &&
		   !m_downloadURL.empty() &&
		   !m_contentType.empty();
}

bool GitHubReleaseAsset::isValid(const GitHubReleaseAsset * a) {
		return a != nullptr && a->isValid();
}

bool GitHubReleaseAsset::operator == (const GitHubReleaseAsset & a) const {
	return m_id == a.m_id &&
		   m_fileSize == a.m_fileSize &&
		   m_createdTimestamp == a.m_createdTimestamp &&
		   m_updatedTimestamp == a.m_updatedTimestamp &&
		   Utilities::areStringsEqual(m_fileName, a.m_fileName) &&
		   Utilities::areStringsEqual(m_label, a.m_label) &&
		   Utilities::areStringsEqual(m_assetURL, a.m_assetURL) &&
		   Utilities::areStringsEqual(m_downloadURL, a.m_downloadURL) &&
		   Utilities::areStringsEqual(m_contentType, a.m_contentType);
}

bool GitHubReleaseAsset::operator != (const GitHubReleaseAsset & a) const {
	return !operator == (a);
}

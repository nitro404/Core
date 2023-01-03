#include "GitHubRelease.h"

#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <spdlog/spdlog.h>

static constexpr const char * JSON_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_RELEASE_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_TAG_NAME_PROPERTY_NAME = "tag_name";
static constexpr const char * JSON_RELEASE_URL_PROPERTY_NAME = "url";
static constexpr const char * JSON_ASSETS_URL_PROPERTY_NAME = "assets_url";
static constexpr const char * JSON_HTML_URL_PROPERTY_NAME = "html_url";
static constexpr const char * JSON_BODY_PROPERTY_NAME = "body";
static constexpr const char * JSON_DRAFT_PROPERTY_NAME = "draft";
static constexpr const char * JSON_PRERELEASE_PROPERTY_NAME = "prerelease";
static constexpr const char * JSON_CREATED_TIMESTAMP_PROPERTY_NAME = "created_at";
static constexpr const char * JSON_PUBLISHED_TIMESTAMP_PROPERTY_NAME = "published_at";
static constexpr const char * JSON_ASSETS_PROPERTY_NAME = "assets";

GitHubRelease::GitHubRelease(uint64_t id, const std::string & releaseName, const std::string & tagName, const std::string & releaseURL, const std::string & assetsURL, const std::string & htmlURL, const std::string & body, bool draft, bool preRelease, std::chrono::time_point<std::chrono::system_clock> createdTimestamp, std::chrono::time_point<std::chrono::system_clock> publishedTimestamp)
	: m_id(id)
	, m_releaseName(releaseName)
	, m_tagName(tagName)
	, m_releaseURL(releaseURL)
	, m_assetsURL(assetsURL)
	, m_htmlURL(htmlURL)
	, m_body(body)
	, m_draft(draft)
	, m_preRelease(preRelease)
	, m_createdTimestamp(createdTimestamp)
	, m_publishedTimestamp(publishedTimestamp) { }

GitHubRelease::GitHubRelease(GitHubRelease && r) noexcept
	: m_id(r.m_id)
	, m_releaseName(std::move(r.m_releaseName))
	, m_tagName(std::move(r.m_tagName))
	, m_releaseURL(std::move(r.m_releaseURL))
	, m_assetsURL(std::move(r.m_assetsURL))
	, m_htmlURL(std::move(r.m_htmlURL))
	, m_body(std::move(r.m_body))
	, m_draft(r.m_draft)
	, m_preRelease(r.m_preRelease)
	, m_createdTimestamp(r.m_createdTimestamp)
	, m_publishedTimestamp(r.m_publishedTimestamp)
	, m_assets(std::move(r.m_assets)) {
	updateParent();
}

GitHubRelease::GitHubRelease(const GitHubRelease & r)
	: m_id(r.m_id)
	, m_releaseName(r.m_releaseName)
	, m_tagName(r.m_tagName)
	, m_releaseURL(r.m_releaseURL)
	, m_assetsURL(r.m_assetsURL)
	, m_htmlURL(r.m_htmlURL)
	, m_body(r.m_body)
	, m_draft(r.m_draft)
	, m_preRelease(r.m_preRelease)
	, m_createdTimestamp(r.m_createdTimestamp)
	, m_publishedTimestamp(r.m_publishedTimestamp) {
	for(std::vector<std::shared_ptr<GitHubReleaseAsset>>::const_iterator i = r.m_assets.begin(); i != r.m_assets.end(); ++i) {
		m_assets.push_back(std::make_shared<GitHubReleaseAsset>(**i));
	}

	updateParent();
}

GitHubRelease & GitHubRelease::operator = (GitHubRelease && r) noexcept {
	if(this != &r) {
		m_id = r.m_id;
		m_releaseName = std::move(r.m_releaseName);
		m_tagName = std::move(r.m_tagName);
		m_releaseURL = std::move(r.m_releaseURL);
		m_assetsURL = std::move(r.m_assetsURL);
		m_htmlURL = std::move(r.m_htmlURL);
		m_body = std::move(r.m_body);
		m_body = std::move(r.m_body);
		m_draft = r.m_draft;
		m_preRelease = r.m_preRelease;
		m_createdTimestamp = r.m_createdTimestamp;
		m_publishedTimestamp = r.m_publishedTimestamp;
		m_assets = std::move(r.m_assets);

		updateParent();
	}

	return *this;
}

GitHubRelease & GitHubRelease::operator = (const GitHubRelease & r) {
	m_assets.clear();

	m_id = r.m_id;
	m_releaseName = r.m_releaseName;
	m_tagName = r.m_tagName;
	m_releaseURL = r.m_releaseURL;
	m_assetsURL = r.m_assetsURL;
	m_htmlURL = r.m_htmlURL;
	m_body = r.m_body;
	m_draft = r.m_draft;
	m_preRelease = r.m_preRelease;
	m_createdTimestamp = r.m_createdTimestamp;
	m_publishedTimestamp = r.m_publishedTimestamp;

	for(std::vector<std::shared_ptr<GitHubReleaseAsset>>::const_iterator i = r.m_assets.begin(); i != r.m_assets.end(); ++i) {
		m_assets.push_back(std::make_shared<GitHubReleaseAsset>(**i));
	}

	updateParent();

	return *this;
}

GitHubRelease::~GitHubRelease() {
	for(std::shared_ptr<GitHubReleaseAsset> & asset : m_assets) {
		asset->setParentGitHubRelease(nullptr);
	}
}

uint64_t GitHubRelease::getID() const {
	return m_id;
}

const std::string & GitHubRelease::getReleaseName() const {
	return m_releaseName;
}

const std::string & GitHubRelease::getTagName() const {
	return m_tagName;
}

const std::string & GitHubRelease::getReleaseURL() const {
	return m_releaseURL;
}

const std::string & GitHubRelease::getAssetsURL() const {
	return m_assetsURL;
}

const std::string & GitHubRelease::getHTMLURL() const {
	return m_htmlURL;
}

const std::string & GitHubRelease::getBody() const {
	return m_body;
}

bool GitHubRelease::isDraft() const {
	return m_draft;
}

bool GitHubRelease::isPreRelease() const {
	return m_preRelease;
}

std::chrono::time_point<std::chrono::system_clock> GitHubRelease::getCreatedTimestamp() const {
	return m_createdTimestamp;
}

std::chrono::time_point<std::chrono::system_clock> GitHubRelease::getPublishedTimestamp() const {
	return m_publishedTimestamp;
}

size_t GitHubRelease::numberOfAssets() const {
	return m_assets.size();
}

bool GitHubRelease::hasAsset(const GitHubReleaseAsset & asset) const {
	return std::find_if(std::begin(m_assets), std::end(m_assets), [&asset](const std::shared_ptr<GitHubReleaseAsset> & currentAsset) {
		return asset == *currentAsset;
	}) != std::end(m_assets);
}

bool GitHubRelease::hasAssetWithID(uint64_t id) const {
	return std::find_if(std::begin(m_assets), std::end(m_assets), [id](const std::shared_ptr<GitHubReleaseAsset> & currentAsset) {
		return id == currentAsset->getID();
	}) != std::end(m_assets);
}

bool GitHubRelease::hasAssetWithFileName(const std::string & fileName, bool caseSensitive) const {
	return std::find_if(std::begin(m_assets), std::end(m_assets), [&fileName, caseSensitive](const std::shared_ptr<GitHubReleaseAsset> & currentAsset) {
		return Utilities::areStringsEqual(fileName, currentAsset->getFileName(), caseSensitive);
	}) != std::end(m_assets);
}

size_t GitHubRelease::indexOfAsset(const GitHubReleaseAsset & asset) const {
	auto assetIterator = std::find_if(std::begin(m_assets), std::end(m_assets), [&asset](const std::shared_ptr<GitHubReleaseAsset> & currentAsset) {
		return asset == *currentAsset;
	});

	if(assetIterator == std::end(m_assets)) {
		return std::numeric_limits<size_t>::max();
	}

	return assetIterator - std::begin(m_assets);
}

size_t GitHubRelease::indexOfAssetWithID(uint64_t id) const {
	auto assetIterator = std::find_if(std::begin(m_assets), std::end(m_assets), [id](const std::shared_ptr<GitHubReleaseAsset> & currentAsset) {
		return id == currentAsset->getID();
	});

	if(assetIterator == std::end(m_assets)) {
		return std::numeric_limits<size_t>::max();
	}

	return assetIterator - std::begin(m_assets);
}

size_t GitHubRelease::indexOfAssetWithFileName(const std::string & fileName, bool caseSensitive) const {
	auto assetIterator = std::find_if(std::begin(m_assets), std::end(m_assets), [&fileName, caseSensitive](const std::shared_ptr<GitHubReleaseAsset> & currentAsset) {
		return Utilities::areStringsEqual(fileName, currentAsset->getFileName(), caseSensitive);
	});

	if(assetIterator == std::end(m_assets)) {
		return std::numeric_limits<size_t>::max();
	}

	return assetIterator - std::begin(m_assets);
}

std::shared_ptr<GitHubReleaseAsset> GitHubRelease::getAsset(size_t index) const {
	if(index >= m_assets.size()) {
		return nullptr;
	}

	return m_assets[index];
}

std::shared_ptr<GitHubReleaseAsset> GitHubRelease::getAssetWithID(uint64_t id) const {
	auto assetIterator = std::find_if(std::begin(m_assets), std::end(m_assets), [id](const std::shared_ptr<GitHubReleaseAsset> & currentAsset) {
		return id == currentAsset->getID();
	});

	if(assetIterator == std::end(m_assets)) {
		return nullptr;
	}

	return *assetIterator;
}

std::shared_ptr<GitHubReleaseAsset> GitHubRelease::getAssetWithFileName(const std::string & fileName, bool caseSensitive) const {
	auto assetIterator = std::find_if(std::begin(m_assets), std::end(m_assets), [&fileName, caseSensitive](const std::shared_ptr<GitHubReleaseAsset> & currentAsset) {
		return Utilities::areStringsEqual(fileName, currentAsset->getFileName(), caseSensitive);
	});

	if(assetIterator == std::end(m_assets)) {
		return nullptr;
	}

	return *assetIterator;
}

void GitHubRelease::updateParent() {
	for(std::shared_ptr<GitHubReleaseAsset> & asset : m_assets) {
		asset->setParentGitHubRelease(this);
	}
}

std::unique_ptr<GitHubRelease> GitHubRelease::parseFrom(const rapidjson::Value & releaseValue) {
	// parse id
	if(!releaseValue.HasMember(JSON_ID_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & idValue = releaseValue[JSON_ID_PROPERTY_NAME];

	if(!idValue.IsUint64()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_ID_PROPERTY_NAME, Utilities::typeToString(idValue.GetType()));
		return nullptr;
	}

	uint64_t id = idValue.GetUint64();

	// parse release name
	if(!releaseValue.HasMember(JSON_RELEASE_NAME_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_RELEASE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & releaseNameValue = releaseValue[JSON_RELEASE_NAME_PROPERTY_NAME];

	if(!releaseNameValue.IsString()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'string'.", JSON_RELEASE_NAME_PROPERTY_NAME, Utilities::typeToString(releaseNameValue.GetType()));
		return nullptr;
	}

	std::string releaseName(Utilities::trimString(releaseNameValue.GetString()));

	// parse tag name
	if(!releaseValue.HasMember(JSON_TAG_NAME_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_TAG_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & tagNameValue = releaseValue[JSON_TAG_NAME_PROPERTY_NAME];

	if(!tagNameValue.IsString()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'string'.", JSON_TAG_NAME_PROPERTY_NAME, Utilities::typeToString(tagNameValue.GetType()));
		return nullptr;
	}

	std::string tagName(Utilities::trimString(tagNameValue.GetString()));

	if(tagName.empty()) {
		spdlog::error("GitHub release '{}' property cannot be empty.", JSON_TAG_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse release url
	if(!releaseValue.HasMember(JSON_RELEASE_URL_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_RELEASE_URL_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & releaseURLValue = releaseValue[JSON_RELEASE_URL_PROPERTY_NAME];

	if(!releaseURLValue.IsString()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'string'.", JSON_RELEASE_URL_PROPERTY_NAME, Utilities::typeToString(releaseURLValue.GetType()));
		return nullptr;
	}

	std::string releaseURL(Utilities::trimString(releaseURLValue.GetString()));

	if(releaseURL.empty()) {
		spdlog::error("GitHub release '{}' property cannot be empty.", JSON_RELEASE_URL_PROPERTY_NAME);
		return nullptr;
	}

	// parse assets url
	if(!releaseValue.HasMember(JSON_ASSETS_URL_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_ASSETS_URL_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & assetsURLValue = releaseValue[JSON_ASSETS_URL_PROPERTY_NAME];

	if(!assetsURLValue.IsString()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'string'.", JSON_ASSETS_URL_PROPERTY_NAME, Utilities::typeToString(assetsURLValue.GetType()));
		return nullptr;
	}

	std::string assetsURL(Utilities::trimString(assetsURLValue.GetString()));

	if(assetsURL.empty()) {
		spdlog::error("GitHub release '{}' property cannot be empty.", JSON_ASSETS_URL_PROPERTY_NAME);
		return nullptr;
	}

	// parse html url
	if(!releaseValue.HasMember(JSON_HTML_URL_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_HTML_URL_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & htmlURLValue = releaseValue[JSON_HTML_URL_PROPERTY_NAME];

	if(!htmlURLValue.IsString()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'string'.", JSON_HTML_URL_PROPERTY_NAME, Utilities::typeToString(htmlURLValue.GetType()));
		return nullptr;
	}

	std::string htmlURL(Utilities::trimString(htmlURLValue.GetString()));

	if(htmlURL.empty()) {
		spdlog::error("GitHub release '{}' property cannot be empty.", JSON_HTML_URL_PROPERTY_NAME);
		return nullptr;
	}

	// parse body
	if(!releaseValue.HasMember(JSON_BODY_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_BODY_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & bodyValue = releaseValue[JSON_BODY_PROPERTY_NAME];

	if(!bodyValue.IsString()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'string'.", JSON_BODY_PROPERTY_NAME, Utilities::typeToString(bodyValue.GetType()));
		return nullptr;
	}

	std::string body(Utilities::trimString(bodyValue.GetString()));

	// parse draft flag
	if(!releaseValue.HasMember(JSON_DRAFT_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property.", JSON_DRAFT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & draftValue = releaseValue[JSON_DRAFT_PROPERTY_NAME];

	if(!draftValue.IsBool()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_DRAFT_PROPERTY_NAME, Utilities::typeToString(draftValue.GetType()));
		return nullptr;
	}

	bool draft = draftValue.GetBool();

	// parse pre-release flag
	if(!releaseValue.HasMember(JSON_PRERELEASE_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property.", JSON_PRERELEASE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & preReleaseValue = releaseValue[JSON_PRERELEASE_PROPERTY_NAME];

	if(!preReleaseValue.IsBool()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_PRERELEASE_PROPERTY_NAME, Utilities::typeToString(preReleaseValue.GetType()));
		return nullptr;
	}

	bool preRelease = preReleaseValue.GetBool();

	// parse created timestamp
	if(!releaseValue.HasMember(JSON_CREATED_TIMESTAMP_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & createdTimestampValue = releaseValue[JSON_CREATED_TIMESTAMP_PROPERTY_NAME];

	if(!createdTimestampValue.IsString()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'string'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(createdTimestampValue.GetType()));
		return nullptr;
	}

	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalCreatedTimestamp(Utilities::parseTimePointFromString(createdTimestampValue.GetString()));

	if(!optionalCreatedTimestamp.has_value()) {
		spdlog::error("GitHub release has an invalid '{}' property value: '{}'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME, createdTimestampValue.GetString());
		return nullptr;
	}

	// parse published timestamp
	if(!releaseValue.HasMember(JSON_PUBLISHED_TIMESTAMP_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_PUBLISHED_TIMESTAMP_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & publishedTimestampValue = releaseValue[JSON_PUBLISHED_TIMESTAMP_PROPERTY_NAME];

	if(!publishedTimestampValue.IsString()) {
		spdlog::error("GitHub release has an invalid '{}' property type: '{}', expected 'string'.", JSON_PUBLISHED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(publishedTimestampValue.GetType()));
		return nullptr;
	}

	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalPublishedTimestamp(Utilities::parseTimePointFromString(publishedTimestampValue.GetString()));

	if(!optionalPublishedTimestamp.has_value()) {
		spdlog::error("GitHub release has an invalid '{}' property value: '{}'.", JSON_PUBLISHED_TIMESTAMP_PROPERTY_NAME, publishedTimestampValue.GetString());
		return nullptr;
	}

	// create the new GitHub release
	std::unique_ptr<GitHubRelease> newRelease(std::make_unique<GitHubRelease>(id, releaseName, tagName, releaseURL, assetsURL, htmlURL, body, draft, preRelease, optionalCreatedTimestamp.value(), optionalPublishedTimestamp.value()));

	// parse assets
	if(!releaseValue.HasMember(JSON_ASSETS_PROPERTY_NAME)) {
		spdlog::error("GitHub release is missing '{}' property'.", JSON_ASSETS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & assetsValue = releaseValue[JSON_ASSETS_PROPERTY_NAME];

	if(!assetsValue.IsArray()) {
		spdlog::error("GitHub release '{}' property has invalid type: '{}', expected 'array'.", JSON_ASSETS_PROPERTY_NAME, Utilities::typeToString(assetsValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<GitHubReleaseAsset> newAsset;

	for(rapidjson::Value::ConstValueIterator i = assetsValue.Begin(); i != assetsValue.End(); ++i) {
		newAsset = std::shared_ptr<GitHubReleaseAsset>(GitHubReleaseAsset::parseFrom(*i).release());

		if(!GitHubReleaseAsset::isValid(newAsset.get())) {
			spdlog::error("Failed to parse GitHub release asset #{}.", newRelease->m_assets.size() + 1);
			return nullptr;
		}

		newAsset->setParentGitHubRelease(newRelease.get());

		if(newRelease->hasAssetWithID(newAsset->getID())) {
			spdlog::error("Encountered duplicate GitHub release asset #{} with ID: '{}'.", newRelease->m_assets.size() + 1, newAsset->getID());
			return nullptr;
		}

		newRelease->m_assets.push_back(newAsset);
	}

	return newRelease;
}

bool GitHubRelease::isValid() const {
	return m_id != 0 &&
		   !m_tagName.empty() &&
		   !m_releaseURL.empty() &&
		   !m_assetsURL.empty() &&
		   !m_htmlURL.empty();
}

bool GitHubRelease::isValid(const GitHubRelease * r) {
		return r != nullptr && r->isValid();
}

bool GitHubRelease::operator == (const GitHubRelease & r) const {
	return m_id == r.m_id &&
		   m_draft == r.m_draft &&
		   m_preRelease == r.m_preRelease &&
		   m_createdTimestamp == r.m_createdTimestamp &&
		   m_publishedTimestamp == r.m_publishedTimestamp &&
		   Utilities::areStringsEqual(m_releaseName, r.m_releaseName) &&
		   Utilities::areStringsEqual(m_tagName, r.m_tagName) &&
		   Utilities::areStringsEqual(m_releaseURL, r.m_releaseURL) &&
		   Utilities::areStringsEqual(m_assetsURL, r.m_assetsURL) &&
		   Utilities::areStringsEqual(m_htmlURL, r.m_htmlURL) &&
		   Utilities::areStringsEqual(m_body, r.m_body) &&
		   m_assets == r.m_assets;
}

bool GitHubRelease::operator != (const GitHubRelease & r) const {
	return !operator == (r);
}

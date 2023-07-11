#include "BitbucketDownloadCollection.h"

#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"

#include <spdlog/spdlog.h>

BitbucketDownloadCollection::BitbucketDownloadCollection() { }

BitbucketDownloadCollection::BitbucketDownloadCollection(BitbucketDownloadCollection && c) noexcept
	: m_downloads(std::move(c.m_downloads)) { }

BitbucketDownloadCollection::BitbucketDownloadCollection(const BitbucketDownloadCollection & c) {
	for(std::vector<std::shared_ptr<BitbucketDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<BitbucketDownload>(**i));
	}
}

BitbucketDownloadCollection & BitbucketDownloadCollection::operator = (BitbucketDownloadCollection && c) noexcept {
	if(this != &c) {
		m_downloads = std::move(c.m_downloads);
	}

	return *this;
}

BitbucketDownloadCollection & BitbucketDownloadCollection::operator = (const BitbucketDownloadCollection & c) {
	m_downloads.clear();

	for(std::vector<std::shared_ptr<BitbucketDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<BitbucketDownload>(**i));
	}

	return *this;
}

BitbucketDownloadCollection::~BitbucketDownloadCollection() { }

size_t BitbucketDownloadCollection::numberOfDownloads() const {
	return m_downloads.size();
}

bool BitbucketDownloadCollection::hasDownload(const BitbucketDownload & download) const {
	return indexOfDownload(download) != std::numeric_limits<size_t>::max();
}

bool BitbucketDownloadCollection::hasDownloadWithFileName(const std::string & fileName, bool caseSensitive) const {
	return indexOfDownloadWithFileName(fileName, caseSensitive) != std::numeric_limits<size_t>::max();
}

size_t BitbucketDownloadCollection::indexOfDownload(const BitbucketDownload & download) const {
	auto downloadIterator = std::find_if(std::begin(m_downloads), std::end(m_downloads), [&download](const std::shared_ptr<BitbucketDownload> & currentDownload) {
		return &download == currentDownload.get();
	});

	if(downloadIterator == std::end(m_downloads)) {
		return std::numeric_limits<size_t>::max();
	}

	return downloadIterator - std::begin(m_downloads);
}

size_t BitbucketDownloadCollection::indexOfDownloadWithFileName(const std::string & fileName, bool caseSensitive) const {
	auto downloadIterator = std::find_if(std::begin(m_downloads), std::end(m_downloads), [&fileName, caseSensitive](const std::shared_ptr<BitbucketDownload> & currentDownload) {
		return Utilities::areStringsEqual(fileName, currentDownload->getFileName(), caseSensitive);
	});

	if(downloadIterator == std::end(m_downloads)) {
		return std::numeric_limits<size_t>::max();
	}

	return downloadIterator - std::begin(m_downloads);
}

size_t BitbucketDownloadCollection::indexOfLatestDownload() const {
	if(m_downloads.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	size_t latestDownloadIndex = 0;

	for(size_t i = 1; i < m_downloads.size(); i++) {
		if(m_downloads[i]->getCreatedTimestamp() > m_downloads[latestDownloadIndex]->getCreatedTimestamp()) {
			latestDownloadIndex = i;
		}
	}

	return latestDownloadIndex;
}

std::shared_ptr<BitbucketDownload> BitbucketDownloadCollection::getDownload(size_t index) const {
	if(index >= m_downloads.size()) {
		return nullptr;
	}

	return m_downloads[index];
}

std::shared_ptr<BitbucketDownload> BitbucketDownloadCollection::getDownloadWithFileName(const std::string & fileName, bool caseSensitive) const {
	return getDownload(indexOfDownloadWithFileName(fileName, caseSensitive));
}

std::shared_ptr<BitbucketDownload> BitbucketDownloadCollection::getLatestDownload() const {
	return getDownload(indexOfLatestDownload());
}

const std::vector<std::shared_ptr<BitbucketDownload>> & BitbucketDownloadCollection::getDownloads() const {
	return m_downloads;
}

std::unique_ptr<BitbucketDownloadCollection> BitbucketDownloadCollection::parseFrom(const rapidjson::Value & downloadCollectionValue) {
	if(!downloadCollectionValue.IsObject()) {
		spdlog::error("Invalid Bitbucket download collection type: '{}', expected 'object'.", Utilities::typeToString(downloadCollectionValue.GetType()));
		return nullptr;
	}

	if(!downloadCollectionValue.HasMember("values")) {
		spdlog::error("Bitbucket download collection is missing 'values' property.");
		return nullptr;
	}

	const rapidjson::Value & downloadCollectionValuesValue = downloadCollectionValue["values"];

	if(!downloadCollectionValuesValue.IsArray()) {
		spdlog::error("Bitbucket download collection is missing 'values' property type: '{}', expected 'array'.", Utilities::typeToString(downloadCollectionValuesValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<BitbucketDownloadCollection> newDownloadCollection(std::make_unique<BitbucketDownloadCollection>());
	std::shared_ptr<BitbucketDownload> newDownload;

	for(rapidjson::Value::ConstValueIterator i = downloadCollectionValuesValue.Begin(); i != downloadCollectionValuesValue.End(); ++i) {
		newDownload = BitbucketDownload::parseFrom(*i);

		if(!BitbucketDownload::isValid(newDownload.get())) {
			spdlog::error("Failed to parse Bitbucket download #{}.", newDownloadCollection->m_downloads.size() + 1);
			return nullptr;
		}

		if(newDownloadCollection->hasDownloadWithFileName(newDownload->getFileName())) {
			spdlog::error("Encountered duplicate Bitbucket download #{} with name: '{}'.", newDownloadCollection->m_downloads.size() + 1, newDownload->getFileName());
			return nullptr;
		}

		newDownloadCollection->m_downloads.push_back(newDownload);
	}

	return newDownloadCollection;
}

bool BitbucketDownloadCollection::isValid() const {
	for(const std::shared_ptr<BitbucketDownload> & download : m_downloads) {
		if(!BitbucketDownload::isValid(download.get())) {
			return false;
		}
	}

	return true;
}

bool BitbucketDownloadCollection::isValid(const BitbucketDownloadCollection * c) {
		return c != nullptr && c->isValid();
}

bool BitbucketDownloadCollection::operator == (const BitbucketDownloadCollection & c) const {
	return m_downloads == c.m_downloads;
}

bool BitbucketDownloadCollection::operator != (const BitbucketDownloadCollection & c) const {
	return !operator == (c);
}

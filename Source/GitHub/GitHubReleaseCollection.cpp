#include "GitHubReleaseCollection.h"

#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"

#include <spdlog/spdlog.h>

GitHubReleaseCollection::GitHubReleaseCollection() { }

GitHubReleaseCollection::GitHubReleaseCollection(GitHubReleaseCollection && c) noexcept
	: m_releases(std::move(c.m_releases)) { }

GitHubReleaseCollection::GitHubReleaseCollection(const GitHubReleaseCollection & c) {
	for(std::vector<std::shared_ptr<GitHubRelease>>::const_iterator i = c.m_releases.begin(); i != c.m_releases.end(); ++i) {
		m_releases.push_back(std::make_shared<GitHubRelease>(**i));
	}
}

GitHubReleaseCollection & GitHubReleaseCollection::operator = (GitHubReleaseCollection && c) noexcept {
	if(this != &c) {
		m_releases = std::move(c.m_releases);
	}

	return *this;
}

GitHubReleaseCollection & GitHubReleaseCollection::operator = (const GitHubReleaseCollection & c) {
	m_releases.clear();

	for(std::vector<std::shared_ptr<GitHubRelease>>::const_iterator i = c.m_releases.begin(); i != c.m_releases.end(); ++i) {
		m_releases.push_back(std::make_shared<GitHubRelease>(**i));
	}

	return *this;
}

GitHubReleaseCollection::~GitHubReleaseCollection() { }

size_t GitHubReleaseCollection::numberOfReleases() const {
	return m_releases.size();
}

bool GitHubReleaseCollection::hasRelease(const GitHubRelease & release) const {
	return std::find_if(std::begin(m_releases), std::end(m_releases), [&release](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return currentRelease.get() == &release;
	}) != std::end(m_releases);
}

bool GitHubReleaseCollection::hasReleaseWithID(uint64_t id) const {
	return std::find_if(std::begin(m_releases), std::end(m_releases), [id](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return id == currentRelease->getID();
	}) != std::end(m_releases);
}

bool GitHubReleaseCollection::hasReleaseWithName(const std::string & releaseName, bool caseSensitive) const {
	return std::find_if(std::begin(m_releases), std::end(m_releases), [&releaseName, caseSensitive](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return Utilities::areStringsEqual(releaseName, currentRelease->getReleaseName(), caseSensitive);
	}) != std::end(m_releases);
}

bool GitHubReleaseCollection::hasReleaseWithTag(const std::string & tagName, bool caseSensitive) const {
	return std::find_if(std::begin(m_releases), std::end(m_releases), [&tagName, caseSensitive](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return Utilities::areStringsEqual(tagName, currentRelease->getTagName(), caseSensitive);
	}) != std::end(m_releases);
}

size_t GitHubReleaseCollection::indexOfRelease(const GitHubRelease & release) const {
	auto releaseIterator = std::find_if(std::begin(m_releases), std::end(m_releases), [&release](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return release == *currentRelease;
	});

	if(releaseIterator == std::end(m_releases)) {
		return std::numeric_limits<size_t>::max();
	}

	return releaseIterator - std::begin(m_releases);
}

size_t GitHubReleaseCollection::indexOfReleaseWithID(uint64_t id) const {
	auto releaseIterator = std::find_if(std::begin(m_releases), std::end(m_releases), [id](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return id == currentRelease->getID();
	});

	if(releaseIterator == std::end(m_releases)) {
		return std::numeric_limits<size_t>::max();
	}

	return releaseIterator - std::begin(m_releases);
}

size_t GitHubReleaseCollection::indexOfReleaseWithName(const std::string & releaseName, bool caseSensitive) const {
	auto releaseIterator = std::find_if(std::begin(m_releases), std::end(m_releases), [&releaseName, caseSensitive](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return Utilities::areStringsEqual(releaseName, currentRelease->getReleaseName(), caseSensitive);
	});

	if(releaseIterator == std::end(m_releases)) {
		return std::numeric_limits<size_t>::max();
	}

	return releaseIterator - std::begin(m_releases);
}

size_t GitHubReleaseCollection::indexOfReleaseWithTag(const std::string & tagName, bool caseSensitive) const {
	auto releaseIterator = std::find_if(std::begin(m_releases), std::end(m_releases), [&tagName, caseSensitive](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return Utilities::areStringsEqual(tagName, currentRelease->getTagName(), caseSensitive);
	});

	if(releaseIterator == std::end(m_releases)) {
		return std::numeric_limits<size_t>::max();
	}

	return releaseIterator - std::begin(m_releases);
}

std::shared_ptr<GitHubRelease> GitHubReleaseCollection::getRelease(size_t index) const {
	if(index >= m_releases.size()) {
		return nullptr;
	}

	return m_releases[index];
}

std::shared_ptr<GitHubRelease> GitHubReleaseCollection::getReleaseWithID(uint64_t id) const {
	auto releaseIterator = std::find_if(std::begin(m_releases), std::end(m_releases), [id](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return id == currentRelease->getID();
	});

	if(releaseIterator == std::end(m_releases)) {
		return nullptr;
	}

	return *releaseIterator;
}

std::shared_ptr<GitHubRelease> GitHubReleaseCollection::getReleaseWithName(const std::string & releaseName, bool caseSensitive) const {
	auto releaseIterator = std::find_if(std::begin(m_releases), std::end(m_releases), [&releaseName, caseSensitive](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return Utilities::areStringsEqual(releaseName, currentRelease->getReleaseName(), caseSensitive);
	});

	if(releaseIterator == std::end(m_releases)) {
		return nullptr;
	}

	return *releaseIterator;
}

std::shared_ptr<GitHubRelease> GitHubReleaseCollection::getReleaseWithTag(const std::string & tagName, bool caseSensitive) const {
	auto releaseIterator = std::find_if(std::begin(m_releases), std::end(m_releases), [&tagName, caseSensitive](const std::shared_ptr<GitHubRelease> & currentRelease) {
		return Utilities::areStringsEqual(tagName, currentRelease->getTagName(), caseSensitive);
	});

	if(releaseIterator == std::end(m_releases)) {
		return nullptr;
	}

	return *releaseIterator;
}

std::unique_ptr<GitHubReleaseCollection> GitHubReleaseCollection::parseFrom(const rapidjson::Value & releaseCollectionValue) {
	if(!releaseCollectionValue.IsArray()) {
		spdlog::error("Invalid GitHub release collection type: '{}', expected 'array'.", Utilities::typeToString(releaseCollectionValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<GitHubReleaseCollection> newReleaseCollection(std::make_unique<GitHubReleaseCollection>());
	std::shared_ptr<GitHubRelease> newRelease;

	for(rapidjson::Value::ConstValueIterator i = releaseCollectionValue.Begin(); i != releaseCollectionValue.End(); ++i) {
		newRelease = std::shared_ptr<GitHubRelease>(GitHubRelease::parseFrom(*i).release());

		if(!GitHubRelease::isValid(newRelease.get())) {
			spdlog::error("Failed to parse GitHub release #{}.", newReleaseCollection->m_releases.size() + 1);
			return nullptr;
		}

		if(newReleaseCollection->hasReleaseWithID(newRelease->getID())) {
			spdlog::error("Encountered duplicate GitHub release #{} with ID: '{}'.", newReleaseCollection->m_releases.size() + 1, newRelease->getID());
			return nullptr;
		}

		newReleaseCollection->m_releases.push_back(newRelease);
	}

	return newReleaseCollection;
}

bool GitHubReleaseCollection::isValid() const {
	for(const std::shared_ptr<GitHubRelease> & release : m_releases) {
		if(!GitHubRelease::isValid(release.get())) {
			return false;
		}
	}

	return true;
}

bool GitHubReleaseCollection::isValid(const GitHubReleaseCollection * c) {
		return c != nullptr && c->isValid();
}

bool GitHubReleaseCollection::operator == (const GitHubReleaseCollection & c) const {
	return m_releases == c.m_releases;
}

bool GitHubReleaseCollection::operator != (const GitHubReleaseCollection & c) const {
	return !operator == (c);
}

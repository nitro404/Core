#ifndef _GITHUB_RELEASE_COLLECTION_H_
#define _GITHUB_RELEASE_COLLECTION_H_

#include "GitHubRelease.h"

#include <memory>
#include <vector>

class GitHubReleaseCollection final {
public:
	GitHubReleaseCollection();
	GitHubReleaseCollection(GitHubReleaseCollection && c) noexcept;
	GitHubReleaseCollection(const GitHubReleaseCollection & c);
	GitHubReleaseCollection & operator = (GitHubReleaseCollection && c) noexcept;
	GitHubReleaseCollection & operator = (const GitHubReleaseCollection & c);
	~GitHubReleaseCollection();

	size_t numberOfReleases() const;
	bool hasRelease(const GitHubRelease & release) const;
	bool hasReleaseWithID(uint64_t id) const;
	bool hasReleaseWithName(const std::string & releaseName, bool caseSensitive = false) const;
	bool hasReleaseWithTag(const std::string & tagName, bool caseSensitive = false) const;
	bool hasReleaseWithTagVersion(const std::string & version, bool caseSensitive = false) const;
	size_t indexOfRelease(const GitHubRelease & release) const;
	size_t indexOfReleaseWithID(uint64_t id) const;
	size_t indexOfReleaseWithName(const std::string & releaseName, bool caseSensitive = false) const;
	size_t indexOfReleaseWithTag(const std::string & tagName, bool caseSensitive = false) const;
	size_t indexOfReleaseWithTagVersion(const std::string & version, bool caseSensitive = false) const;
	size_t indexOfLatestRelease() const;
	std::shared_ptr<GitHubRelease> getRelease(size_t index) const;
	std::shared_ptr<GitHubRelease> getReleaseWithID(uint64_t id) const;
	std::shared_ptr<GitHubRelease> getReleaseWithName(const std::string & releaseName, bool caseSensitive = false) const;
	std::shared_ptr<GitHubRelease> getReleaseWithTag(const std::string & tagName, bool caseSensitive = false) const;
	std::shared_ptr<GitHubRelease> getReleaseWithTagVersion(const std::string & version, bool caseSensitive = false) const;
	std::shared_ptr<GitHubRelease> getLatestRelease() const;
	std::vector<std::string> getReleaseNames() const;
	std::vector<std::string> getReleaseTagNames() const;

	static std::unique_ptr<GitHubReleaseCollection> parseFrom(const rapidjson::Value & releaseCollectionValue, bool includePreReleases = false, bool includeDrafts = false);

	bool isValid() const;
	static bool isValid(const GitHubReleaseCollection * c);

	bool operator == (const GitHubReleaseCollection & c) const;
	bool operator != (const GitHubReleaseCollection & c) const;

private:
	std::vector<std::shared_ptr<GitHubRelease>> m_releases;
};

#endif // _GITHUB_RELEASE_COLLECTION_H_

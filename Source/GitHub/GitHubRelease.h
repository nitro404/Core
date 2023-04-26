#ifndef _GITHUB_RELEASE_H_
#define _GITHUB_RELEASE_H_

#include "GitHubReleaseAsset.h"

#include <rapidjson/document.h>

#include <memory>
#include <string>
#include <vector>

class GitHubRelease final {
public:
	GitHubRelease(uint64_t id, const std::string & releaseName, const std::string & tagName, const std::string & releaseURL, const std::string & assetsURL, const std::string & htmlURL, const std::string & body, bool draft, bool preRelease, std::chrono::time_point<std::chrono::system_clock> createdTimestamp, std::chrono::time_point<std::chrono::system_clock> publishedTimestamp);
	GitHubRelease(GitHubRelease && r) noexcept;
	GitHubRelease(const GitHubRelease & r);
	GitHubRelease & operator = (GitHubRelease && r) noexcept;
	GitHubRelease & operator = (const GitHubRelease & r);
	~GitHubRelease();

	uint64_t getID() const;
	const std::string & getReleaseName() const;
	const std::string & getTagName() const;
	const std::string & getReleaseURL() const;
	const std::string & getAssetsURL() const;
	const std::string & getHTMLURL() const;
	const std::string & getBody() const;
	bool isDraft() const;
	bool isPreRelease() const;
	std::chrono::time_point<std::chrono::system_clock> getCreatedTimestamp() const;
	std::chrono::time_point<std::chrono::system_clock> getPublishedTimestamp() const;

	size_t numberOfAssets() const;
	bool hasAsset(const GitHubReleaseAsset & asset) const;
	bool hasAssetWithID(uint64_t id) const;
	bool hasAssetWithFileName(const std::string & fileName, bool caseSensitive = false) const;
	size_t indexOfAsset(const GitHubReleaseAsset & asset) const;
	size_t indexOfAssetWithID(uint64_t id) const;
	size_t indexOfAssetWithFileName(const std::string & fileName, bool caseSensitive = false) const;
	std::shared_ptr<GitHubReleaseAsset> getAsset(size_t index) const;
	std::shared_ptr<GitHubReleaseAsset> getAssetWithID(uint64_t id) const;
	std::shared_ptr<GitHubReleaseAsset> getAssetWithFileName(const std::string & fileName, bool caseSensitive = false) const;

	static std::unique_ptr<GitHubRelease> parseFrom(const rapidjson::Value & releaseValue);

	bool isValid() const;
	static bool isValid(const GitHubRelease * r);

	bool operator == (const GitHubRelease & r) const;
	bool operator != (const GitHubRelease & r) const;

protected:
	void updateParent();

private:
	uint64_t m_id;
	std::string m_releaseName;
	std::string m_tagName;
	std::string m_releaseURL;
	std::string m_assetsURL;
	std::string m_htmlURL;
	std::string m_body;
	bool m_draft;
	bool m_preRelease;
	std::chrono::time_point<std::chrono::system_clock> m_createdTimestamp;
	std::chrono::time_point<std::chrono::system_clock> m_publishedTimestamp;
	std::vector<std::shared_ptr<GitHubReleaseAsset>> m_assets;
};

#endif // _GITHUB_RELEASE_H_

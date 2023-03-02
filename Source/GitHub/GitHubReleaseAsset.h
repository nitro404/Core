#ifndef _GITHUB_RELEASE_ASSET_H_
#define _GITHUB_RELEASE_ASSET_H_

#include <rapidjson/document.h>

#include <chrono>
#include <memory>
#include <string>

class GitHubRelease;

class GitHubReleaseAsset final {
	friend class GitHubRelease;

public:
	GitHubReleaseAsset(uint64_t id, const std::string & fileName, uint64_t fileSize, const std::string & label,  const std::string & assetURL, const std::string & downloadURL, const std::string & contentType, std::chrono::time_point<std::chrono::system_clock> createdTimestamp, std::chrono::time_point<std::chrono::system_clock> updatedTimestamp);
	GitHubReleaseAsset(GitHubReleaseAsset && a) noexcept;
	GitHubReleaseAsset(const GitHubReleaseAsset & a);
	GitHubReleaseAsset & operator = (GitHubReleaseAsset && a) noexcept;
	GitHubReleaseAsset & operator = (const GitHubReleaseAsset & a);
	~GitHubReleaseAsset();

	uint64_t getID() const;
	const std::string & getFileName() const;
	uint64_t getFileSize() const;
	const std::string & getAssetURL() const;
	const std::string & getDownloadURL() const;
	const std::string & getLabel() const;
	const std::string & getContentType() const;
	std::chrono::time_point<std::chrono::system_clock> getCreatedTimestamp() const;
	std::chrono::time_point<std::chrono::system_clock> getUpdatedTimestamp() const;

	const GitHubRelease * getParentGitHubRelease() const;

	static std::unique_ptr<GitHubReleaseAsset> parseFrom(const rapidjson::Value & assetValue);

	bool isValid() const;
	static bool isValid(const GitHubReleaseAsset * a);

	bool operator == (const GitHubReleaseAsset & a) const;
	bool operator != (const GitHubReleaseAsset & a) const;

protected:
	void setParentGitHubRelease(const GitHubRelease * release);

private:
	uint64_t m_id;
	std::string m_fileName;
	uint64_t m_fileSize;
	std::string m_label;
	std::string m_assetURL;
	std::string m_downloadURL;
	std::string m_contentType;
	std::chrono::time_point<std::chrono::system_clock> m_createdTimestamp;
	std::chrono::time_point<std::chrono::system_clock> m_updatedTimestamp;

	const GitHubRelease * m_parentGitHubRelease;
};

#endif // _GITHUB_RELEASE_ASSET_H_

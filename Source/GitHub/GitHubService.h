#ifndef _GITHUB_SERVICE_H_
#define _GITHUB_SERVICE_H_

#include "GitHubReleaseCollection.h"
#include "Singleton/Singleton.h"

#include <memory>
#include <optional>
#include <string>

class GitHubService final : public Singleton<GitHubService> {
public:
	GitHubService();
	virtual ~GitHubService();

	std::unique_ptr<GitHubRelease> getLatestRelease(const std::string & repositoryURL) const;
	std::unique_ptr<GitHubRelease> getLatestRelease(const std::string & repositoryName, const std::string & organizationName) const;
	std::unique_ptr<GitHubReleaseCollection> getReleases(const std::string & repositoryURL) const;
	std::unique_ptr<GitHubReleaseCollection> getReleases(const std::string & repositoryName, const std::string & organizationName) const;

private:
	struct RepositoryInformation {
		std::string repositoryName;
		std::string organizationName;
	};

	static std::optional<RepositoryInformation> parseRepositoryURL(const std::string & repositoryURL);

	bool m_initialized;

	GitHubService(const GitHubService&) = delete;
	GitHubService(GitHubService&&) noexcept = delete;
	const GitHubService& operator = (const GitHubService&) = delete;
	const GitHubService& operator = (GitHubService&&) noexcept = delete;
};

#endif // _GITHUB_SERVICE_H_

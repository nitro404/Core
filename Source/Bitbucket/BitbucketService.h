#ifndef _BITBUCKET_SERVICE_H_
#define _BITBUCKET_SERVICE_H_

#include "BitbucketDownloadCollection.h"
#include "Singleton/Singleton.h"

#include <memory>
#include <optional>
#include <string>

class BitbucketService final : public Singleton<BitbucketService> {
public:
	BitbucketService();
	virtual ~BitbucketService();

	std::shared_ptr<BitbucketDownload> getLatestDownload(const std::string & repositoryURL) const;
	std::shared_ptr<BitbucketDownload> getLatestDownload(const std::string & repositoryName, const std::string & organizationName) const;
	std::unique_ptr<BitbucketDownloadCollection> getDownloads(const std::string & repositoryURL) const;
	std::unique_ptr<BitbucketDownloadCollection> getDownloads(const std::string & repositoryName, const std::string & organizationName) const;

private:
	struct RepositoryInformation {
		std::string repositoryName;
		std::string organizationName;
	};

	static std::optional<RepositoryInformation> parseRepositoryURL(const std::string & repositoryURL);

	bool m_initialized;

	BitbucketService(const BitbucketService &) = delete;
	BitbucketService(BitbucketService &&) noexcept = delete;
	const BitbucketService & operator = (const BitbucketService &) = delete;
	const BitbucketService & operator = (BitbucketService &&) noexcept = delete;
};

#endif // _BITBUCKET_SERVICE_H_

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
	~BitbucketService() override;

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

	BitbucketService(const BitbucketService &) = delete;
	const BitbucketService & operator = (const BitbucketService &) = delete;
};

#endif // _BITBUCKET_SERVICE_H_

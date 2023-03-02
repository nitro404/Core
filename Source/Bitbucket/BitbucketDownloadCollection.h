#ifndef _BITBUCKET_DOWNLOAD_COLLECTION_H_
#define _BITBUCKET_DOWNLOAD_COLLECTION_H_

#include "BitbucketDownload.h"

#include <memory>
#include <vector>

class BitbucketDownloadCollection final {
public:
	BitbucketDownloadCollection();
	BitbucketDownloadCollection(BitbucketDownloadCollection && c) noexcept;
	BitbucketDownloadCollection(const BitbucketDownloadCollection & c);
	BitbucketDownloadCollection & operator = (BitbucketDownloadCollection && c) noexcept;
	BitbucketDownloadCollection & operator = (const BitbucketDownloadCollection & c);
	~BitbucketDownloadCollection();

	size_t numberOfDownloads() const;
	bool hasDownload(const BitbucketDownload & release) const;
	bool hasDownloadWithFileName(const std::string & fileName, bool caseSensitive = false) const;
	size_t indexOfDownload(const BitbucketDownload & release) const;
	size_t indexOfDownloadWithFileName(const std::string & fileName, bool caseSensitive = false) const;
	size_t indexOfLatestDownload() const;
	std::shared_ptr<BitbucketDownload> getDownload(size_t index) const;
	std::shared_ptr<BitbucketDownload> getDownloadWithFileName(const std::string & fileName, bool caseSensitive = false) const;
	std::shared_ptr<BitbucketDownload> getLatestDownload() const;
	const std::vector<std::shared_ptr<BitbucketDownload>> & getDownloads() const;

	static std::unique_ptr<BitbucketDownloadCollection> parseFrom(const rapidjson::Value & releaseCollectionValue);

	bool isValid() const;
	static bool isValid(const BitbucketDownloadCollection * c);

	bool operator == (const BitbucketDownloadCollection & c) const;
	bool operator != (const BitbucketDownloadCollection & c) const;

private:
	std::vector<std::shared_ptr<BitbucketDownload>> m_downloads;
};

#endif // _BITBUCKET_DOWNLOAD_COLLECTION_H_

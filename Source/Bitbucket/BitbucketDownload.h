#ifndef _BITBUCKET_DOWNLOAD_H_
#define _BITBUCKET_DOWNLOAD_H_

#include <rapidjson/document.h>

#include <chrono>
#include <memory>
#include <string>

class BitbucketDownload final {
public:
	BitbucketDownload(const std::string & fileName, uint64_t fileSize, const std::string & downloadURL, std::chrono::time_point<std::chrono::system_clock> createdTimestamp);
	BitbucketDownload(BitbucketDownload && a) noexcept;
	BitbucketDownload(const BitbucketDownload & a);
	BitbucketDownload & operator = (BitbucketDownload && a) noexcept;
	BitbucketDownload & operator = (const BitbucketDownload & a);
	~BitbucketDownload();

	const std::string & getFileName() const;
	uint64_t getFileSize() const;
	const std::string & getDownloadURL() const;
	std::chrono::time_point<std::chrono::system_clock> getCreatedTimestamp() const;

	static std::unique_ptr<BitbucketDownload> parseFrom(const rapidjson::Value & downloadValue);

	bool isValid() const;
	static bool isValid(const BitbucketDownload * a);

	bool operator == (const BitbucketDownload & a) const;
	bool operator != (const BitbucketDownload & a) const;

private:
	std::string m_fileName;
	uint64_t m_fileSize;
	std::string m_downloadURL;
	std::chrono::time_point<std::chrono::system_clock> m_createdTimestamp;
};

#endif // _BITBUCKET_DOWNLOAD_H_

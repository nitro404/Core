#include "BitbucketService.h"

#include "Network/HTTPService.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <regex>

static const std::string BITBUCKET_API_BASE_URL("https://api.bitbucket.org/2.0");
static const std::string BITBUCKET_DOWNLOADS_API_URL_TEMPLATE(Utilities::joinPaths(BITBUCKET_API_BASE_URL, "repositories/{}/{}/downloads"));

BitbucketService::BitbucketService() { }

BitbucketService::~BitbucketService() { }

std::shared_ptr<BitbucketDownload> BitbucketService::getLatestDownload(const std::string & repositoryURL) const {
	std::optional<RepositoryInformation> repositoryInfo(BitbucketService::parseRepositoryURL(repositoryURL));

	if(!repositoryInfo.has_value()) {
		spdlog::error("Failed to parse repository URL: '{}'.", repositoryURL);
		return nullptr;
	}

	return getLatestDownload(repositoryInfo->repositoryName, repositoryInfo->organizationName);
}

std::shared_ptr<BitbucketDownload> BitbucketService::getLatestDownload(const std::string & repositoryName, const std::string & organizationName) const {
	std::unique_ptr<BitbucketDownloadCollection> downloads(getDownloads(repositoryName, organizationName));

	if(downloads == nullptr || downloads->numberOfDownloads() == 0) {
		return nullptr;
	}

	return downloads->getLatestDownload();
}

std::unique_ptr<BitbucketDownloadCollection> BitbucketService::getDownloads(const std::string & repositoryURL) const {
	std::optional<RepositoryInformation> repositoryInfo(BitbucketService::parseRepositoryURL(repositoryURL));

	if(!repositoryInfo.has_value()) {
		spdlog::error("Failed to parse repository URL: '{}'.", repositoryURL);
		return nullptr;
	}

	return getDownloads(repositoryInfo->repositoryName, repositoryInfo->organizationName);
}

std::unique_ptr<BitbucketDownloadCollection> BitbucketService::getDownloads(const std::string & repositoryName, const std::string & organizationName) const {
	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::error("Failed to retrieve '{}' downloads information, HTTP service is not initialized!");
		return nullptr;
	}

	std::string downloadsURL(fmt::format(BITBUCKET_DOWNLOADS_API_URL_TEMPLATE, organizationName, repositoryName));

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, downloadsURL));

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response == nullptr || response->isFailure()) {
		spdlog::error("Failed to retrieve '{}' downloads information with error: {}", repositoryName, response != nullptr ? response->getErrorMessage() : "Invalid request.");
		return nullptr;
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to retrieve '{}' downloads information ({}{})!", repositoryName, response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return nullptr;
	}

	std::unique_ptr<rapidjson::Document> downloadCollectionDocument(response->getBodyAsJSON());

	if(downloadCollectionDocument == nullptr) {
		spdlog::error("Failed to parse '{}' downloads JSON data.", repositoryName);
		return nullptr;
	}

	std::unique_ptr<BitbucketDownloadCollection> downloadCollection(BitbucketDownloadCollection::parseFrom(*downloadCollectionDocument));

	if(!BitbucketDownloadCollection::isValid(downloadCollection.get())) {
		spdlog::error("Failed to parse '{}' downloads from JSON data.", repositoryName);
		return nullptr;
	}

	return downloadCollection;
}

std::optional<BitbucketService::RepositoryInformation> BitbucketService::parseRepositoryURL(const std::string & repositoryURL) {
	static const std::regex BITBUCKET_REPOSITORY_URL_REGEX("https?://(?:www.)?bitbucket.org/([^/]+)/(.+)$");

	if(repositoryURL.empty()) {
		return {};
	}

	std::string formattedRepositoryURL(Utilities::trimString(repositoryURL));

	std::smatch repositoryURLParts;
	std::regex_search(formattedRepositoryURL, repositoryURLParts, BITBUCKET_REPOSITORY_URL_REGEX);

	if(repositoryURLParts.size() != 3) {
		return {};
	}

	return RepositoryInformation{ repositoryURLParts[2].str(), repositoryURLParts[1].str() };
}

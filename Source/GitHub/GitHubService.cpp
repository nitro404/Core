#include "GitHubService.h"

#include "Network/HTTPService.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

#include <fmt/args.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <regex>

static const std::string GITHUB_API_BASE_URL("https://api.github.com");
static const std::string GITHUB_RELEASES_API_URL_TEMPLATE(Utilities::joinPaths(GITHUB_API_BASE_URL, "repos/{}/{}/releases"));
static const std::string GITHUB_LATEST_RELEASE_API_URL_TEMPLATE(Utilities::joinPaths(GITHUB_RELEASES_API_URL_TEMPLATE, "latest"));

GitHubService::GitHubService() { }

GitHubService::~GitHubService() { }

std::unique_ptr<GitHubRelease> GitHubService::getLatestRelease(const std::string & repositoryURL) const {
	std::optional<RepositoryInformation> repositoryInfo(GitHubService::parseRepositoryURL(repositoryURL));

	if(!repositoryInfo.has_value()) {
		spdlog::error("Failed to parse repository URL: '{}'.", repositoryURL);
		return nullptr;
	}

	return getLatestRelease(repositoryInfo->repositoryName, repositoryInfo->organizationName);
}

std::unique_ptr<GitHubRelease> GitHubService::getLatestRelease(const std::string & repositoryName, const std::string & organizationName) const {
	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::error("Failed to retrieve latest '{}' release information, HTTP service is not initialized!");
		return nullptr;
	}

	fmt::dynamic_format_arg_store<fmt::format_context> githubLatestReleaseApiUrlArguments;
	githubLatestReleaseApiUrlArguments.push_back(organizationName);
	githubLatestReleaseApiUrlArguments.push_back(repositoryName);

	std::string latestReleaseURL(fmt::vformat(GITHUB_LATEST_RELEASE_API_URL_TEMPLATE, githubLatestReleaseApiUrlArguments));

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, latestReleaseURL));

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response == nullptr || response->isFailure()) {
		spdlog::error("Failed to retrieve latest '{}' release information with error: {}", repositoryName, response != nullptr ? response->getErrorMessage() : "Invalid request.");
		return nullptr;
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to retrieve latest '{}' release information ({}{})!", repositoryName, response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return nullptr;
	}

	std::unique_ptr<rapidjson::Document> latestReleaseDocument(response->getBodyAsJSON());

	if(latestReleaseDocument == nullptr) {
		spdlog::error("Failed to parse latest '{}' release JSON data.", repositoryName);
		return nullptr;
	}

	std::unique_ptr<GitHubRelease> latestRelease(GitHubRelease::parseFrom(*latestReleaseDocument));

	if(!GitHubRelease::isValid(latestRelease.get())) {
		spdlog::error("Failed to parse latest '{}' release from JSON data.", repositoryName);
		return nullptr;
	}

	return latestRelease;
}

std::unique_ptr<GitHubReleaseCollection> GitHubService::getReleases(const std::string & repositoryURL, bool includePreReleases, bool includeDrafts) const {
	std::optional<RepositoryInformation> repositoryInfo(GitHubService::parseRepositoryURL(repositoryURL));

	if(!repositoryInfo.has_value()) {
		spdlog::error("Failed to parse repository URL: '{}'.", repositoryURL);
		return nullptr;
	}

	return getReleases(repositoryInfo->repositoryName, repositoryInfo->organizationName, includePreReleases, includeDrafts);
}

std::unique_ptr<GitHubReleaseCollection> GitHubService::getReleases(const std::string & repositoryName, const std::string & organizationName, bool includePreReleases, bool includeDrafts) const {
	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::error("Failed to retrieve '{}' releases information, HTTP service is not initialized!", repositoryName);
		return nullptr;
	}

	fmt::dynamic_format_arg_store<fmt::format_context> githubReleasesApiUrlArguments;
	githubReleasesApiUrlArguments.push_back(organizationName);
	githubReleasesApiUrlArguments.push_back(repositoryName);

	std::string releasesURL(fmt::vformat(GITHUB_RELEASES_API_URL_TEMPLATE, githubReleasesApiUrlArguments));

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, releasesURL));

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response == nullptr || response->isFailure()) {
		spdlog::error("Failed to retrieve '{}' releases information with error: {}", repositoryName, response != nullptr ? response->getErrorMessage() : "Invalid request.");
		return nullptr;
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to retrieve '{}' releases information ({}{})!", repositoryName, response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return nullptr;
	}

	std::unique_ptr<rapidjson::Document> releaseCollectionDocument(response->getBodyAsJSON());

	if(releaseCollectionDocument == nullptr) {
		spdlog::error("Failed to parse '{}' releases JSON data.", repositoryName);
		return nullptr;
	}

	std::unique_ptr<GitHubReleaseCollection> releaseCollection(GitHubReleaseCollection::parseFrom(*releaseCollectionDocument, includePreReleases, includeDrafts));

	if(!GitHubReleaseCollection::isValid(releaseCollection.get())) {
		spdlog::error("Failed to parse '{}' releases from JSON data.", repositoryName);
		return nullptr;
	}

	return releaseCollection;
}

std::optional<GitHubService::RepositoryInformation> GitHubService::parseRepositoryURL(const std::string & repositoryURL) {
	static const std::regex GITHUB_REPOSITORY_URL_REGEX("https?://(?:www.)?github.com/([^/]+)/(.+)$");

	if(repositoryURL.empty()) {
		return {};
	}

	std::string formattedRepositoryURL(Utilities::trimString(repositoryURL));

	std::smatch repositoryURLParts;
	std::regex_search(formattedRepositoryURL, repositoryURLParts, GITHUB_REPOSITORY_URL_REGEX);

	if(repositoryURLParts.size() != 3) {
		return {};
	}

	return RepositoryInformation{ repositoryURLParts[2].str(), repositoryURLParts[1].str() };
}

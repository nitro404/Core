#include "LibraryInformation.h"

#include "Core.h"
#include "Utilities/StringUtilities.h"

#include <boost/version.hpp>
#include <bzlib.h>
#include <cryptopp/cryptlib.h>
#include <curl/curl.h>
#include <dmc_unrar/dmc_unrar.h>
#include <fmt/core.h>
#include <libssh2.h>
#include <zip.h>
#include <lzma.h>
#include <magic_enum.hpp>
#include <rapidjson/rapidjson.h>
#include <SevenZip/C/7zVersion.h>
#include <spdlog/spdlog.h>
#include <tidy/tidy.h>
#include <tinyxml2.h>
#include <zlib.h>
#include <zstd.h>

#include <sstream>

LibraryInformation::LibraryInformation()
	: m_libraryInformation({
		{ "Core", CORE_VERSION, CORE_COMMIT_HASH },
		{ "7-Zip", MY_VERSION },
		{ "Boost", fmt::format("{}.{}.{}", BOOST_VERSION / 10000, (BOOST_VERSION % 10000) / 100, BOOST_VERSION % 100) },
		{ "BZip", BZ2_bzlibVersion() },
		{ "Crypto++", fmt::format("{}.{}.{}", CryptoPP::LibraryVersion() / 100, (CryptoPP::LibraryVersion() % 100) / 10, CryptoPP::LibraryVersion() % 10) },
		{ "cURL", LIBCURL_VERSION },
		{ "dmc_unrar", DMC_UNRAR_VERSION },
		{ "fmt", fmt::format("{}.{}.{}", FMT_VERSION / 10000, (FMT_VERSION % 10000) / 100, FMT_VERSION % 100) },
		{ "LibLZMA", lzma_version_string() },
		{ "LibSSH2", LIBSSH2_VERSION },
		{ "LibZIP", zip_libzip_version() },
		{ "Magic Enum C++", fmt::format("{}.{}.{}", MAGIC_ENUM_VERSION_MAJOR, MAGIC_ENUM_VERSION_MINOR, MAGIC_ENUM_VERSION_PATCH) },
		{ "RapidJSON", RAPIDJSON_VERSION_STRING },
		{ "spdlog", fmt::format("{}.{}.{}", SPDLOG_VERSION / 10000, (SPDLOG_VERSION % 10000) / 100, SPDLOG_VERSION % 100) },
		{ "TidyHTML5", tidyLibraryVersion(), tidyReleaseDate() },
		{ "TinyXML2", fmt::format("{}.{}.{}", TINYXML2_MAJOR_VERSION, TINYXML2_MINOR_VERSION, TINYXML2_PATCH_VERSION) },
		{ "ZLib", zlibVersion() },
		{ "Zstandard", ZSTD_versionString() }
	}) { }

LibraryInformation::~LibraryInformation() { }

size_t LibraryInformation::numberOfLibraries() const {
	return m_libraryInformation.size();
}

bool LibraryInformation::hasLibrary(const std::string & name) const {
	return std::find_if(m_libraryInformation.cbegin(), m_libraryInformation.cend(), [&name](const VersionData & versionData) {
		return Utilities::areStringsEqualIgnoreCase(versionData.name, name);
	}) != m_libraryInformation.cend();
}

size_t LibraryInformation::indexOfLibrary(const std::string & name) const {
	std::vector<VersionData>::const_iterator libraryInformationIterator = std::find_if(m_libraryInformation.cbegin(), m_libraryInformation.cend(), [&name](const VersionData & versionData) {
		return Utilities::areStringsEqualIgnoreCase(versionData.name, name);
	});

	if(libraryInformationIterator == m_libraryInformation.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return libraryInformationIterator - m_libraryInformation.cbegin();
}

const LibraryInformation::VersionData * LibraryInformation::getLibraryVersionData(size_t index) const {
	if(index >= m_libraryInformation.size()) {
		return nullptr;
	}

	return &m_libraryInformation[index];
}

const LibraryInformation::VersionData * LibraryInformation::getLibraryVersionData(const std::string & name) const {
	return getLibraryVersionData(indexOfLibrary(name));
}

std::string LibraryInformation::getLibraryVersion(size_t index) const {
	const VersionData * libraryVersionData = getLibraryVersionData(index);

	if(libraryVersionData == nullptr) {
		return {};
	}

	return libraryVersionData->name;
}

std::string LibraryInformation::getLibraryVersion(const std::string & name) const {
	return getLibraryVersion(indexOfLibrary(name));
}

const std::vector<LibraryInformation::VersionData> & LibraryInformation::getLibraryInformation() const {
	return m_libraryInformation;
}

std::string LibraryInformation::getLibraryInformationString() const {
	std::stringstream libraryInformationStream;

	for(const VersionData & versionData : m_libraryInformation) {
		if(libraryInformationStream.tellp() != 0) {
			libraryInformationStream << "\n";
		}

		libraryInformationStream << versionData.name << ": " << versionData.version;

		if(!versionData.extra.empty()) {
			libraryInformationStream << " (" << versionData.extra << ")";
		}
	}

	return libraryInformationStream.str();
}

bool LibraryInformation::addLibrary(const std::string & name, const std::string & version, const std::string & extra) {
	if(name.empty() || version.empty() || hasLibrary(name)) {
		return false;
	}

	m_libraryInformation.push_back({name, version, extra});

	return true;
}

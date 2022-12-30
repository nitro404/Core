#include "LibraryInformation.h"

#include "Core.h"

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

namespace LibraryInformation {

	const std::vector<VersionData> & getLibraryInformation() {
		static std::vector<VersionData> s_libraryInformation;

		if(s_libraryInformation.empty()) {
			s_libraryInformation = {
				{ "Core", CORE_VERSION, CORE_COMMIT_HASH },
				{ "7-Zip", MY_VERSION },
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
			};
		}

		return s_libraryInformation;
	}

	const std::string & getLibraryInformationString() {
		static std::string s_libraryInformation;

		if(s_libraryInformation.empty()) {
			std::stringstream versionStream;
			const std::vector<VersionData> & libraryInformationData = getLibraryInformation();

			for(const VersionData & versionData : libraryInformationData) {
				if(versionStream.tellp() != 0) {
					versionStream << "\n";
				}

				versionStream << versionData.name << ": " << versionData.version;

				if(!versionData.extra.empty()) {
					versionStream << " (" << versionData.extra << ")";
				}
			}

			s_libraryInformation = versionStream.str();
		}

		return s_libraryInformation;
	}

}

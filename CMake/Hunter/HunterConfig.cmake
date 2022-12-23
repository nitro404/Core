include_guard()

hunter_config(BZip2
	URL "https://github.com/Telefrag-Software/bzip2/archive/refs/heads/hunter-1.0.8.zip"
	VERSION "1.0.8"
	SHA1 "1aff69cc4d5ad8af639901fda1642d45217ec9d8"
)

hunter_config(CryptoPP
	URL "https://github.com/Telefrag-Software/cryptopp/archive/refs/heads/hunter-8.7.0.1.zip"
	VERSION "8.7.0.1"
	SHA1 "38c2ae2d5d5f196277454f528993d76824001317"
)

hunter_config(CURL
	URL "https://github.com/Telefrag-Software/curl/archive/refs/heads/hunter-7.87.0.0.zip"
	VERSION "7.87.0.0"
	SHA1 "1665f94819bef1e21e2622771e13acf527e3fc25"
	CMAKE_ARGS
		CURL_USE_OPENSSL=ON
		BUILD_SHARED_LIBS=OFF
)

hunter_config(date
	URL "https://github.com/Telefrag-Software/date/archive/refs/heads/hunter-3.0.1.2.zip"
	VERSION "3.0.1.2"
	SHA1 "5046f9fc6b9b0b4432807659c1f21c4638da0fcb"
	CMAKE_ARGS
		BUILD_TZ_LIB=ON
		USE_SYSTEM_TZ_DB=OFF
		MANUAL_TZ_DB=ON
)

hunter_config(dmc_unrar
	URL "https://github.com/Telefrag-Software/dmc_unrar/archive/refs/heads/hunter-1.7.0.zip"
	VERSION "1.7.0"
	SHA1 "985425d6268954b043abcdeea7f947e14d8d6290"
)

hunter_config(double-conversion
	URL "https://github.com/Telefrag-Software/double-conversion/archive/refs/heads/hunter-3.2.1.1.zip"
	VERSION "3.2.1.1"
	SHA1 "cc123cefcc3361d979734d9c2bbc51631bd7b81b"
)

hunter_config(fmt
	URL "https://codeload.github.com/fmtlib/fmt/zip/tags/9.1.0"
	VERSION "9.1.0"
	SHA1 "acc663874d9f9362b1b2f3cab49b3d9fe254854c"
)

hunter_config(GTest
	URL "https://github.com/Telefrag-Software/googletest/archive/refs/heads/hunter-1.11.0.1.zip"
	VERSION "1.11.0.1"
	SHA1 "399764392feff6f6482aff04d40c7ed33ffa0dc5"
)

hunter_config(liblzma
	URL "https://github.com/Telefrag-Software/xz/archive/refs/heads/hunter-5.4.0.zip"
	VERSION "5.4.0"
	SHA1 "2347b787e6626f6ffa6baac0a36ceb5dc5439309"
	CMAKE_ARGS
		BUILD_TESTING=OFF
)

hunter_config(libzip
	URL "https://github.com/Telefrag-Software/libzip/archive/refs/heads/hunter-1.9.2.1.zip"
	VERSION "1.9.2.1"
	SHA1 "a8981f9ac94783fbd9927e2a01a4f2dab17bdfcf"
	CMAKE_ARGS
		BUILD_TOOLS=OFF
		BUILD_REGRESS=OFF
		BUILD_EXAMPLES=OFF
		BUILD_DOC=OFF
)

hunter_config(magic_enum
	URL "https://github.com/Telefrag-Software/magic_enum/archive/refs/heads/hunter-0.7.3.1.zip"
	VERSION "0.7.3.1"
	SHA1 "3e7181abeb7b2778a18fa4ae73d8a57db6d5ff1c"
)

hunter_config(RapidJSON
	URL "https://github.com/Telefrag-Software/rapidjson/archive/refs/heads/hunter-1.1.0.1.zip"
	VERSION "1.1.0.1"
	SHA1 "626bc8a7d738c0ab5648f1c7c86e06ed7d760c1a"
)

hunter_config(SevenZip
	URL "https://github.com/Telefrag-Software/7-Zip/archive/refs/heads/hunter-21.7.0.1.zip"
	VERSION "21.7.0.1"
	SHA1 "68e5db5849b3b09d3612324fae99e582144a69dd"
)

hunter_config(spdlog
	URL "https://github.com/Telefrag-Software/spdlog/archive/refs/heads/hunter-1.11.0.0.zip"
	VERSION "1.11.0.0"
	SHA1 "5fdb178952d99d6bcc9d9e77ca443817ba4b0ea6"
)

hunter_config(tidy
	URL "https://github.com/Telefrag-Software/tidy-html5/archive/refs/heads/hunter-5.8.0.2.zip"
	VERSION "5.8.0.2"
	SHA1 "51d9e9329db1eeaf1ffafd075f6d4370b5b9d81d"
	CMAKE_ARGS
		BUILD_SHARED_LIB=OFF
		SUPPORT_CONSOLE_APP=OFF
		DISABLE_DEBUG_LOG=ON
)

hunter_config(tinyxml2
	URL "https://github.com/Telefrag-Software/tinyxml2/archive/refs/heads/hunter-9.0.0.1.zip"
	VERSION "9.0.0.1"
	SHA1 "594b6b1e75477ad7d769713f0b59edeb929614c7"
)

hunter_config(ZLIB
	URL "https://github.com/Telefrag-Software/zlib/archive/refs/heads/hunter-1.2.13.1.zip"
	VERSION "1.2.13.1"
	SHA1 "1189eb8d6a0d8f5b2cd676f09661f4864cc85896"
)

hunter_config(zstd
	URL "https://codeload.github.com/facebook/zstd/zip/tags/v1.5.2"
	VERSION "1.5.2"
	SHA1 "f52fc3920828076655386f14da4e3bf4dba6467e"
)

# cURL Dependencies
hunter_config(c-ares
	VERSION "1.14.0-p0"
)

hunter_config(Libssh2
	VERSION "1.9.0-p0"
)

hunter_config(OpenSSL
	VERSION "1.1.1o"
)

include("${CMAKE_SOURCE_DIR}/CMake/Hunter/HunterConfig.cmake" OPTIONAL)

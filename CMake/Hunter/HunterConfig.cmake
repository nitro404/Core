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
	URL "https://github.com/Telefrag-Software/curl/archive/refs/heads/hunter-7.81.0.6.zip"
	VERSION "7.81.0.6"
	SHA1 "22ea0a7d6e770da57926ed43ac1c2c8dea3d4a89"
	CMAKE_ARGS
		CURL_USE_OPENSSL=ON
		BUILD_SHARED_LIBS=OFF
)

hunter_config(date
	URL "https://github.com/Telefrag-Software/date/archive/refs/heads/hunter-3.0.1.1.zip"
	VERSION "3.0.1.1"
	SHA1 "18061fdb7b6ddcc09aff2c28a5cc6ff0d4cd651f"
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
	URL "https://github.com/Telefrag-Software/double-conversion/archive/refs/heads/hunter-3.1.5.zip"
	VERSION "3.1.5"
	SHA1 "663b03adac706fde59ef50fe549cdf516c1532a2"
)

hunter_config(fmt
	URL "https://github.com/Telefrag-Software/fmt/archive/refs/heads/hunter-8.0.1.zip"
	VERSION "8.0.1"
	SHA1 "b5cd7cca9bda1490d76843dad25f9c585b37a5cb"
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
	URL "https://github.com/Telefrag-Software/libzip/archive/refs/heads/hunter-1.8.0.2.zip"
	VERSION "1.8.0.2"
	SHA1 "fe21a3d2e4724774506c431c7d1e2095629fb68e"
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
	URL "https://github.com/Telefrag-Software/spdlog/archive/refs/heads/hunter-1.8.5.1.zip"
	VERSION "1.8.5.1"
	SHA1 "37aa29d005449fa4f3cef7264b3796f72b59acc3"
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

include("${CMAKE_SOURCE_DIR}/CMake/Hunter/HunterConfig.cmake" OPTIONAL)

include_guard()

hunter_config(7Zip
	URL "https://github.com/Telefrag-Software/7-Zip/archive/refs/heads/hunter-21.7.0.zip"
	VERSION "21.7.0"
	SHA1 "832fac1b5776d83f879fd4924afbec9d70494f64"
)

hunter_config(BZip2
	URL "https://github.com/Telefrag-Software/bzip2/archive/refs/heads/hunter-1.0.6.zip"
	VERSION "1.0.6"
	SHA1 "957a9a1e137840af0f75dc6798f349d15cbfdb38"
)

hunter_config(CryptoPP
	URL "https://github.com/Telefrag-Software/cryptopp/archive/refs/heads/hunter-8.5.0.1.zip"
	VERSION "8.5.0.1"
	SHA1 "20a451fd89437f65ecadfc7cf5ed75064441835b"
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

hunter_config(libzip
	URL "https://github.com/Telefrag-Software/libzip/archive/refs/heads/hunter-1.8.0.2.zip"
	VERSION "1.8.0.2"
	SHA1 "fe21a3d2e4724774506c431c7d1e2095629fb68e"
)

hunter_config(lzma
	URL "https://github.com/Telefrag-Software/lzma/archive/refs/heads/hunter-5.2.3.zip"
	VERSION "5.2.3"
	SHA1 "6d8c01e2895034bb91bf90711bd5f3dab3607bf1"
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

hunter_config(spdlog
	URL "https://github.com/Telefrag-Software/spdlog/archive/refs/heads/hunter-1.8.5.1.zip"
	VERSION "1.8.5.1"
	SHA1 "37aa29d005449fa4f3cef7264b3796f72b59acc3"
)

hunter_config(tidy
	URL "https://github.com/Telefrag-Software/tidy-html5/archive/refs/heads/hunter-5.8.0.1.zip"
	VERSION "5.8.0.1"
	SHA1 "a6f7024f92f2f39c62fc0ca04a25489f8d76e1ea"
	CMAKE_ARGS
		BUILD_SHARED_LIB=OFF
		SUPPORT_CONSOLE_APP=OFF
		DISABLE_DEBUG_LOG=ON
)

hunter_config(tinyxml2
	URL "https://github.com/Telefrag-Software/tinyxml2/archive/refs/heads/hunter-9.0.0.zip"
	VERSION "9.0.0"
	SHA1 "0da3ec832b72e1cc3b27aff8327ab0f43ecbb484"
)

hunter_config(ZLIB
	URL "https://github.com/Telefrag-Software/zlib/archive/refs/heads/hunter-1.2.12.1.zip"
	VERSION "1.2.12.1"
	SHA1 "b41c9126bf63dfa53fbb496e24dcceb30ddabee4"
)

include("${CMAKE_SOURCE_DIR}/CMake/Hunter/HunterConfig.cmake" OPTIONAL)

include_guard()

if(WIN32)
	set(BUILD_7Z_CPP_LIB ON)
endif()

hunter_config(Boost
	VERSION "1.80.0"
)

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
	URL "https://github.com/Telefrag-Software/date/archive/refs/heads/hunter-3.0.1.3.zip"
	VERSION "3.0.1.3"
	SHA1 "f513791222d7430ffff0345312f2e285da0cc6f4"
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
	URL "https://codeload.github.com/fmtlib/fmt/zip/tags/10.1.1"
	VERSION "10.1.1.0"
	SHA1 "ace30ce9270669c63317f43e10917a85a9acfdcc"
)

hunter_config(GTest
	URL "https://codeload.github.com/google/googletest/zip/tags/v1.14.0"
	VERSION "1.14.0.0"
	SHA1 "713bdcaf2ed0050e582d7c2a05cb9a35523ddf4e"
)

hunter_config(liblzma
	URL "https://codeload.github.com/tukaani-project/xz/zip/tags/v5.4.5"
	VERSION "5.4.5"
	SHA1 "99d49eecca208460832e422148c0cb1f3c8ff7cd"
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
	URL "https://codeload.github.com/Neargye/magic_enum/zip/tags/v0.9.5"
	VERSION "0.9.5"
	SHA1 "e6164e735dd2fe169142954d9529c340701b8669"
)

hunter_config(RapidJSON
	URL "https://github.com/Telefrag-Software/rapidjson/archive/refs/heads/hunter-1.1.0.3.zip"
	VERSION "1.1.0.3"
	SHA1 "e587aa956425051d3936d14cd0db597f0cf6ea16"
)

hunter_config(SevenZip
	URL "https://github.com/Telefrag-Software/7-Zip/archive/refs/heads/hunter-21.7.0.2.zip"
	VERSION "21.7.0.2"
	SHA1 "0286aef04c00a928c264008188eda25efb23c389"
	CMAKE_ARGS
		BUILD_CPP_LIB=${BUILD_7Z_CPP_LIB}
)

hunter_config(spdlog
	URL "https://github.com/Telefrag-Software/spdlog/archive/refs/heads/hunter-1.12.0.0.zip"
	VERSION "1.12.0.0"
	SHA1 "b1b289fe0e7da8d972151cab28b76cb39ba7a9a4"
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
	URL "https://github.com/Telefrag-Software/zlib/archive/refs/heads/hunter-1.3.0.0.zip"
	VERSION "1.3.0.0"
	SHA1 "553c130c05f514e5db8b4ae623347e717674a4d1"
)

hunter_config(zstd
	URL "https://codeload.github.com/facebook/zstd/zip/tags/v1.5.5"
	VERSION "1.5.5.0"
	SHA1 "f9ba91e59dfbd100e04fbf737f58c7c2fffb0918"
)

# cURL Dependencies
hunter_config(c-ares
	VERSION "1.24.0"
)

hunter_config(Libssh2
	VERSION "1.9.0-p0"
)

hunter_config(OpenSSL
	VERSION "1.1.1t"
)

include("${CMAKE_SOURCE_DIR}/CMake/Hunter/HunterConfig.cmake" OPTIONAL)

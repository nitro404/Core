include_guard()

if(WIN32)
	set(BUILD_7Z_CPP_LIB ON)
else()
	set(BUILD_7Z_CPP_LIB OFF)
endif()

hunter_config(Boost
	URL "https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.bz2"
	VERSION "1.87"
	SHA1 "acf19e9d53f0b6db8e85452978eb9c731ef8b865"
)

# Brotli is used by cURL.
hunter_config(brotli
	URL "https://github.com/Telefrag-Software/brotli/archive/refs/heads/hunter-1.2.0.1.zip"
	VERSION "1.2.0.1"
	SHA1 "77118cfb19c5fa3267d13f680dc7d45c5ff51e82"
	CMAKE_ARGS
		BUILD_SHARED_LIBS=OFF
		BROTLI_BUILD_TOOLS=OFF
		BROTLI_DISABLE_TESTS=ON
		ENABLE_COVERAGE=OFF
)

# BZip2 is used by LibZIP and PCRE2.
hunter_config(BZip2
	URL "https://github.com/Telefrag-Software/bzip2/archive/refs/heads/hunter-1.1.0.1.zip"
	VERSION "1.1.0.1"
	SHA1 "e5c848657e22f7e59165feee281b31aa15d2268d"
	CMAKE_ARGS
		ENABLE_APP=OFF
		ENABLE_TESTS=OFF
		ENABLE_DOCS=OFF
		ENABLE_EXAMPLES=OFF
		ENABLE_STATIC_LIB=ON
		ENABLE_SHARED_LIB=OFF
		ENABLE_STATIC_LIB_IS_PIC=ON
)

hunter_config(cryptopp
	URL "https://github.com/Telefrag-Software/cryptopp/archive/refs/heads/hunter-8.9.0.2.zip"
	VERSION "8.9.0.2"
	SHA1 "6e7c41078d8d5dd62824fd3876c7d1d421a8e08e"
	CMAKE_ARGS
		BUILD_STATIC=ON
		BUILD_SHARED=OFF
		BUILD_TESTING=OFF
		BUILD_DOCUMENTATION=OFF
)

hunter_config(CURL
	URL "https://github.com/Telefrag-Software/curl/archive/refs/heads/hunter-8.5.0.zip"
	VERSION "8.5.0.0"
	SHA1 "9d82ea2685a3402c5385f700bbd9832f85112148"
	CMAKE_ARGS
		CURL_USE_OPENSSL=ON
		CURL_ZSTD=ON
		BUILD_SHARED_LIBS=OFF
)

hunter_config(date
	URL "https://github.com/Telefrag-Software/date/archive/refs/heads/hunter-3.0.4.1.zip"
	VERSION "3.0.4.1"
	SHA1 "5372d0daefa652aa57ba1a1ea1d7bfaeb0eb7ebf"
	CMAKE_ARGS
		BUILD_TZ_LIB=ON
		USE_SYSTEM_TZ_DB=OFF
		MANUAL_TZ_DB=ON
		BUILD_SHARED_LIBS=OFF
		ENABLE_DATE_TESTING=OFF
		DISABLE_STRING_VIEW=OFF
		COMPILE_WITH_C_LOCALE=OFF
		ENABLE_DATE_INSTALL=ON
)

hunter_config(dmc_unrar
	URL "https://github.com/Telefrag-Software/dmc_unrar/archive/refs/heads/hunter-1.7.0.1.zip"
	VERSION "1.7.0.1"
	SHA1 "8b86af06e1c8379fff207516deb19bc26a8bde89"
)

hunter_config(double-conversion
	URL "https://github.com/Telefrag-Software/double-conversion/archive/refs/heads/hunter-3.4.0.zip"
	VERSION "3.4.0"
	SHA1 "581925b4ddf707094f24a66ebaf50458240d5fdf"
	CMAKE_ARGS
		BUILD_SHARED_LIBS=OFF
		BUILD_TESTING=OFF
)

# {fmt} is used by spdlog.
hunter_config(fmt
	URL "https://codeload.github.com/fmtlib/fmt/zip/tags/12.1.0"
	VERSION "12.1.0"
	SHA1 "2d2580111a263b772bfbc266dbdfa902cb1a8bf9"
	CMAKE_ARGS
		FMT_PEDANTIC=OFF
		FMT_WERROR=OFF
		FMT_DOC=OFF
		FMT_INSTALL=ON
		FMT_TEST=OFF
		FMT_FUZZ=OFF
		FMT_CUDA_TEST=OFF
		FMT_OS=ON
		FMT_MODULE=OFF
		FMT_SYSTEM_HEADERS=OFF
		FMT_UNICODE=ON
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
	URL "https://github.com/Telefrag-Software/libzip/archive/refs/heads/hunter-1.10.1.0.zip"
	VERSION "1.10.1.0"
	SHA1 "4c652502b252484f888f1dba3175481494647c70"
	CMAKE_ARGS
		ENABLE_BZIP2=ON
		ENABLE_LZMA=ON
		ENABLE_ZSTD=ON
		BUILD_TOOLS=OFF
		BUILD_REGRESS=OFF
		BUILD_EXAMPLES=OFF
		BUILD_DOC=OFF
		BUILD_OSSFUZZ=OFF
)

hunter_config(magic_enum
	URL "https://github.com/Telefrag-Software/magic_enum/archive/refs/heads/hunter-0.9.7.1.zip"
	VERSION "0.9.7.1"
	SHA1 "fdb0b26231cab124211fddff0bb7bb003f697f04"
	CMAKE_ARGS
		MAGIC_ENUM_OPT_BUILD_EXAMPLES=OFF
		MAGIC_ENUM_OPT_BUILD_TESTS=OFF
		MAGIC_ENUM_OPT_INSTALL=ON
		MAGIC_ENUM_OPT_TEST_INSTALLED_VERSION=OFF
		MAGIC_ENUM_OPT_TEST_INSTALLED_VERSION_PKGCONFIG=OFF
		MAGIC_ENUM_OPT_INSTALL_PACKAGE_XML=OFF
)

hunter_config(RapidJSON
	URL "https://github.com/Telefrag-Software/rapidjson/archive/refs/heads/hunter-1.1.0.3.zip"
	VERSION "1.1.0.3"
	SHA1 "e587aa956425051d3936d14cd0db597f0cf6ea16"
)

hunter_config(SevenZip
	URL "https://github.com/Telefrag-Software/7-Zip/archive/refs/heads/hunter-21.7.0.3.zip"
	VERSION "21.7.0.3"
	SHA1 "7c1c5c7fbea2da100ca99ae3cdbdfc263a1457b0"
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

# c-ares is used by cURL.
hunter_config(c-ares
	URL "https://codeload.github.com/c-ares/c-ares/zip/tags/v1.34.6"
	VERSION "1.34.6"
	SHA1 "2470860765112049c3aba8a79b76f438ae73b1ba"
	CMAKE_ARGS
		CARES_STATIC=ON
		CARES_SHARED=OFF
		CARES_INSTALL=ON
		CARES_STATIC_PIC=OFF
		CARES_BUILD_TESTS=OFF
		CARES_BUILD_CONTAINER_TESTS=OFF
		CARES_BUILD_TOOLS=OFF
		CARES_SYMBOL_HIDING=ON
		CARES_THREADS=ON
		CARES_COVERAGE=OFF
)

hunter_config(Libssh2
	VERSION "1.9.0-p0"
)

hunter_config(OpenSSL
	VERSION "1.1.1t"
)

include("${CMAKE_SOURCE_DIR}/CMake/Hunter/HunterConfig.cmake" OPTIONAL)

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

hunter_config(BZip2
	URL "https://github.com/Telefrag-Software/bzip2/archive/refs/heads/hunter-1.1.0.1.zip"
	VERSION "1.1.0.1"
	SHA1 "ea652959e7cbeb0b71b59f3573e2aa96f32782ce"
)

hunter_config(cryptopp
	URL "https://github.com/Telefrag-Software/cryptopp/archive/refs/heads/hunter-8.9.0.2.zip"
	VERSION "8.9.0.2"
	SHA1 "a57d50f23fc1e95b6873a1cef9806e3c4558b31a"
	CMAKE_ARGS
		BUILD_STATIC=ON
		BUILD_SHARED=OFF
		BUILD_TESTING=OFF
		BUILD_DOCUMENTATION=OFF
)

hunter_config(CURL
	URL "https://github.com/Telefrag-Software/curl/archive/refs/heads/hunter-8.17.0.zip"
	VERSION "8.17.0.0"
	SHA1 "3ccb092b86e8cdf90e16589542154dcd40aab200"
	CMAKE_ARGS
		CURL_USE_OPENSSL=ON
		CURL_ZSTD=ON
		BUILD_SHARED_LIBS=OFF
)

hunter_config(date
	URL "https://github.com/Telefrag-Software/date/archive/refs/heads/hunter-3.0.4.1.zip"
	VERSION "3.0.4.1"
	SHA1 "06107c325f2befbed6e06f504b56e95d358737ca"
	CMAKE_ARGS
		BUILD_TZ_LIB=ON
		USE_SYSTEM_TZ_DB=OFF
		MANUAL_TZ_DB=ON
)

hunter_config(dmc_unrar
	URL "https://github.com/Telefrag-Software/dmc_unrar/archive/refs/heads/hunter-1.7.0.1.zip"
	VERSION "1.7.0.1"
	SHA1 "8b86af06e1c8379fff207516deb19bc26a8bde89"
)

hunter_config(double-conversion
	URL "https://github.com/Telefrag-Software/double-conversion/archive/refs/heads/hunter-3.4.0.zip"
	VERSION "3.4.0"
	SHA1 "4c93a5593409e986e0b8173a1249db00af62f2e5"
)

hunter_config(fmt
	URL "https://codeload.github.com/fmtlib/fmt/zip/tags/12.1.0"
	VERSION "12.1.0"
	SHA1 "2d2580111a263b772bfbc266dbdfa902cb1a8bf9"
)

hunter_config(GTest
	URL "https://codeload.github.com/google/googletest/zip/tags/v1.17.0"
	VERSION "1.17.0.0"
	SHA1 "c9cd451f89c5d5869ba7fa88efa3c9d91db628ad"
)

hunter_config(liblzma
	URL "https://codeload.github.com/tukaani-project/xz/zip/tags/v5.8.2"
	VERSION "5.8.2"
	SHA1 "078a77cfee78937449a9f159c6d35fe81806f268"
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
	URL "https://codeload.github.com/Neargye/magic_enum/zip/tags/v0.9.7"
	VERSION "0.9.7"
	SHA1 "4a8e4814f679f10654a9cb7a9d1316c820493c42"
)

hunter_config(RapidJSON
	URL "https://github.com/Telefrag-Software/rapidjson/archive/refs/heads/hunter-1.1.0.4.zip"
	VERSION "1.1.0.4"
	SHA1 "eeec831cf68c2e4aca66d32b0df04e73154fcfd4"
)

hunter_config(SevenZip
	URL "https://github.com/Telefrag-Software/7-Zip/archive/refs/heads/hunter-21.7.0.3.zip"
	VERSION "21.7.0.3"
	SHA1 "7c1c5c7fbea2da100ca99ae3cdbdfc263a1457b0"
	CMAKE_ARGS
		BUILD_CPP_LIB=${BUILD_7Z_CPP_LIB}
)

hunter_config(spdlog
	URL "https://github.com/Telefrag-Software/spdlog/archive/refs/heads/hunter-1.17.0.0.zip"
	VERSION "1.17.0.0"
	SHA1 "f8573e5513ce93e988510692a70e507ebecbfd28"
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
	URL "https://codeload.github.com/leethomason/tinyxml2/zip/tags/11.0.0"
	VERSION "11.0.0"
	SHA1 "0f35e6e7196a8f244a08ee5808bd8a3d575b46b7"
)

hunter_config(ZLIB
	URL "https://codeload.github.com/madler/zlib/zip/tags/v1.3.1.2"
	VERSION "1.3.1.2"
	SHA1 "7a93d599baf1c6983357e241f2571bf6860512b0"
	CMAKE_ARGS
		ZLIB_BUILD_TESTING=OFF
		ZLIB_BUILD_SHARED=OFF
		ZLIB_BUILD_STATIC=ON
		ZLIB_BUILD_MINIZIP=OFF
		ZLIB_INSTALL=ON
		ZLIB_PREFIX=OFF
)

hunter_config(zstd
	URL "https://codeload.github.com/facebook/zstd/zip/tags/v1.5.7"
	VERSION "1.5.7.0"
	SHA1 "8bd73c3d2bf87f0c033299f5d05fb4bde2480ad1"
)

# cURL Dependencies
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
	URL "https://codeload.github.com/libssh2/libssh2/zip/tags/libssh2-1.11.1"
	VERSION "1.11.1"
	SHA1 "9ce8506fed4742280730b15c7d9b64d0daef3955"
	CMAKE_ARGS
		BUILD_STATIC_LIBS=ON
		BUILD_SHARED_LIBS=OFF
		LIBSSH2_DISABLE_INSTALL=OFF
		BUILD_EXAMPLES=OFF
		BUILD_TESTING=OFF
		BUILD_OSSFUZZ=OFF
		HIDE_SYMBOLS=ON
		LIBSSH2_NO_DEPRECATED=OFF
		LIBSSH2_CLANG_TIDY=OFF
		ENABLE_ECDSA_WINCNG=OFF
		LINT=OFF
		LIBSSH2_BUILD_DOCS=OFF
)

hunter_config(OpenSSL
	VERSION "1.1.1w"
)

include("${CMAKE_SOURCE_DIR}/CMake/Hunter/HunterConfig.cmake" OPTIONAL)

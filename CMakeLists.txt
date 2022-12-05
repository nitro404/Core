cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(CORE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
set(_SOURCE_DIRECTORY "Source")
set(_CORE_FILE_PATH "${CORE_DIRECTORY}/${_SOURCE_DIRECTORY}/Core.h")

include(Modules/Debug)
include(Modules/GetGitCommitHash)
include(Hunter/HunterInit)

project(Core VERSION 0.0.1 LANGUAGES C CXX)

include(ThirdPartyLibraries)

if(NOT (CMAKE_GENERATOR MATCHES "Visual Studio") AND NOT CMAKE_BUILD_TYPE)
	message(FATAL_ERROR "Must set 'CMAKE_BUILD_TYPE' for single configuration generators.")
endif()

get_git_commit_hash()

set(CORE_GIT_COMMIT_HASH ${GIT_COMMIT_HASH})

configure_file("${_CORE_FILE_PATH}.in" "${_CORE_FILE_PATH}" @ONLY)

include(SourceFiles)

list(APPEND SOURCE_FILES ${SOURCE_FILES_${PLATFORM_UPPER}})

list(TRANSFORM SOURCE_FILES PREPEND "${_SOURCE_DIRECTORY}/")

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${_SOURCE_DIRECTORY}
)

target_compile_definitions(${PROJECT_NAME}
	PUBLIC
		${PLATFORM_UPPER}
		NOMINMAX
		CRYPTOPP_ENABLE_NAMESPACE_WEAK=1
)

target_link_libraries(
	${PROJECT_NAME}
		PUBLIC
			7Zip::7Zip
			BZip2::bz2
			cryptopp-static
			CURL::libcurl
			date::date
			date::date-tz
			double-conversion::double-conversion
			fmt::fmt
			GTest::gtest
			libzip::zip
			lzma::lzma
			magic_enum::magic_enum
			RapidJSON::rapidjson
			spdlog::spdlog
			tidy::tidy-static
			tinyxml2::tinyxml2
			ZLIB::zlib
)

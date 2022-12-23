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

add_library(${PROJECT_NAME} STATIC ${CORE_ALL_FILES})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${_SOURCE_DIRECTORY}
)

target_compile_definitions(${PROJECT_NAME}
	PUBLIC
		${PLATFORM_UPPER}
		CRYPTOPP_ENABLE_NAMESPACE_WEAK=1
)

if(WIN32)
	target_compile_definitions(${PROJECT_NAME}
		PUBLIC
			NOMINMAX
	)
endif()

target_link_libraries(
	${PROJECT_NAME}
		PUBLIC
			BZip2::bz2
			cryptopp-static
			CURL::libcurl
			date::date
			date::date-tz
			dmc_unrar::dmc_unrar
			double-conversion::double-conversion
			fmt::fmt
			LibLZMA::LibLZMA
			Libssh2::libssh2
			libzip::zip
			magic_enum::magic_enum
			RapidJSON::rapidjson
			SevenZip::SevenZip
			spdlog::spdlog
			tidy::tidy-static
			tinyxml2::tinyxml2
			ZLIB::zlib
)

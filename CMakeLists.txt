cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(CORE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
set(_SOURCE_DIRECTORY "Source")

include(Modules/Debug)
include(Hunter/HunterInit)

project(Core LANGUAGES C CXX)

include(ThirdPartyLibraries)

if(NOT (CMAKE_GENERATOR MATCHES "Visual Studio") AND NOT CMAKE_BUILD_TYPE)
	message(FATAL_ERROR "Must set 'CMAKE_BUILD_TYPE' for single configuration generators.")
endif()

include(SourceList.cmake)

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
)

target_link_libraries(
	${PROJECT_NAME}
		PUBLIC
			cryptopp-static
			double-conversion::double-conversion
			fmt::fmt
			GTest::gtest
			libzip::zip
			magic_enum::magic_enum
			RapidJSON::rapidjson
			spdlog::spdlog
			tinyxml2::tinyxml2
			ZLIB::zlib
)

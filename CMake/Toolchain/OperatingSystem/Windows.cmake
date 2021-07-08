include_guard()

include(${CMAKE_CURRENT_LIST_DIR}/../Modules/SetPlatform.cmake)

set_platform(WINDOWS)

set(CMAKE_GENERATOR "Visual Studio 16 2019")
set(CMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_GENERATOR_TOOLSET "v142")

include_guard()

include(${CMAKE_CURRENT_LIST_DIR}/Modules/SetPlatform.cmake)

set(VS2022 TRUE)
set_platform(WINDOWS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_GENERATOR "Visual Studio 17 2022")
set(CMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_GENERATOR_TOOLSET "v143")

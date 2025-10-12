include_guard()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(WINDOWS)
	set(CMAKE_GENERATOR_PLATFORM "x64")
	set(CMAKE_GENERATOR_TOOLSET "v143")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(__DEBUG)
endif()

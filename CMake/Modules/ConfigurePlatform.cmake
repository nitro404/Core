include_guard()

include(Modules/SetPlatform)

if(WIN32)
	set_platform(WINDOWS)
elseif(UNIX)
	set_platform(LINUX)
else()
	message(FATAL_ERROR "Unsupported platform.")
endif()

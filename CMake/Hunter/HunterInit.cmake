include_guard()

include(${CORE_DIRECTORY}/CMake/Hunter/HunterGate.cmake)

HunterGate(
	URL "https://github.com/cpp-pm/hunter/archive/v0.26.6.tar.gz"
	SHA1 "e70c29f878f5d5f5cdf1b9ccd628fb872e8624a8"
	FILEPATH "${CORE_DIRECTORY}/CMake/Hunter/HunterConfig.cmake"
)

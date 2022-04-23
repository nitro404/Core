include_guard()

include(${CORE_DIRECTORY}/CMake/Hunter/HunterGate.cmake)

HunterGate(
	URL "https://github.com/cpp-pm/hunter/archive/v0.24.1.tar.gz"
	SHA1 "4942227a6e6f5e64414c55b97ef98609de199d18"
	FILEPATH "${CORE_DIRECTORY}/CMake/Hunter/HunterConfig.cmake"
)

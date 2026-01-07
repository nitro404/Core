include_guard()

include(${CORE_DIRECTORY}/CMake/Hunter/HunterGate.cmake)

HunterGate(
	URL "https://github.com/Telefrag-Software/hunter/archive/v0.26.6.1.tar.gz"
	SHA1 "bbb82011e198711f75a108ccaf216e0ff849fbb5"
	FILEPATH "${CORE_DIRECTORY}/CMake/Hunter/HunterConfig.cmake"
)

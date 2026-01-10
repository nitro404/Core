include_guard()

include(${CORE_DIRECTORY}/CMake/Hunter/HunterGate.cmake)

HunterGate(
	URL "https://github.com/Telefrag-Software/hunter/archive/v0.26.6.3.tar.gz"
	SHA1 "40bdfd3a9e55ee01270d4bd85cdca7656fdaadfe"
	FILEPATH "${CORE_DIRECTORY}/CMake/Hunter/HunterConfig.cmake"
)

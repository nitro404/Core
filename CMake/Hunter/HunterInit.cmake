include_guard()

include(${CORE_DIRECTORY}/CMake/Hunter/HunterGate.cmake)

HunterGate(
	URL "https://github.com/Telefrag-Software/hunter/archive/v0.26.10.1.tar.gz"
	SHA1 "aa02af4d52e99b2bfb764325e52e1bbcd3dde9ac"
	FILEPATH "${CORE_DIRECTORY}/CMake/Hunter/HunterConfig.cmake"
)

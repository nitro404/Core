include_guard()

include(${CORE_DIRECTORY}/CMake/Hunter/HunterGate.cmake)

HunterGate(
	URL "https://github.com/Telefrag-Software/hunter/archive/v0.26.6.2.tar.gz"
	SHA1 "fb1b71f35090488880e941cd7447c5df5a5ecded"
	FILEPATH "${CORE_DIRECTORY}/CMake/Hunter/HunterConfig.cmake"
)

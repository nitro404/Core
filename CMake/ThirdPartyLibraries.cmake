include_guard()

hunter_add_package(CryptoPP)
hunter_add_package(CURL)
hunter_add_package(date)
hunter_add_package(double-conversion)
hunter_add_package(fmt)
hunter_add_package(GTest)
hunter_add_package(libzip)
hunter_add_package(magic_enum)
hunter_add_package(RapidJSON)
hunter_add_package(spdlog)
hunter_add_package(tinyxml2)
hunter_add_package(ZLIB)

find_package(CryptoPP CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)
find_package(double-conversion CONFIG REQUIRED)
find_package(fmt REQUIRED CONFIG)
find_package(GTest CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(ZLIB CONFIG REQUIRED)

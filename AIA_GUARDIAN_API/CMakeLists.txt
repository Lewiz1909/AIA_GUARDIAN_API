cmake_minimum_required(VERSION 3.15)
project(AIA_GUARDIAN_API LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

find_package(nlohmann_json CONFIG REQUIRED)

include_directories(include)

add_executable(AIA_GUARDIAN_API
    src/config_manager.cpp
    main.cpp
)

target_link_libraries(AIA_GUARDIAN_API PRIVATE nlohmann_json::nlohmann_json)

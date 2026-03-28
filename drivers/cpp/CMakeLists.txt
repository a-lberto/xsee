cmake_minimum_required(VERSION 3.15)

set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "")
set(VCPKG_HOST_TRIPLET "x64-mingw-static" CACHE STRING "")

project(XseEngine)

set(CMAKE_CXX_STANDARD 17)

# Find your packages
find_package(LibXml2 REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(main main.cpp)

# Link them
target_link_libraries(main PRIVATE 
    LibXml2::LibXml2 
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
)


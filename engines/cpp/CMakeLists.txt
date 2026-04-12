cmake_minimum_required(VERSION 3.15)

project(XSEE)

set(CMAKE_CXX_STANDARD 17)

find_package(LibXml2 REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# xsee/engines/cpp -> ../..
set(XSEE_ROOT_DIR "${PROJECT_SOURCE_DIR}/../..")
set(EXAMPLE_TEST_DIR "${XSEE_ROOT_DIR}/tests/example")

include_directories(include)
add_executable(xsee src/main.cpp src/HtmlDocument.cpp)

target_include_directories(xsee PRIVATE src)

target_link_libraries(xsee PRIVATE 
    LibXml2::LibXml2 
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
)

enable_testing()

# add_test(
#     NAME IntegrationTest
#     COMMAND main "${EXAMPLE_TEST_DIR}/input.html" --yaml "${EXAMPLE_TEST_DIR}/example.xsee.yaml"
#     WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
# )
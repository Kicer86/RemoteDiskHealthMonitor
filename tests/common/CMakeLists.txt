
add_executable(commonTests
    ProbeStatusTests.cpp
    SerializationTests.cpp
)

target_include_directories(commonTests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external/nlohmann-json/single_include
)

target_link_libraries(commonTests
    common
    gmock_main
)

add_test(
    NAME commonTests
    COMMAND commonTests
)

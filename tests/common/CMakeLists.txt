
find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(commonTests
    ProbeStatusTests.cpp
    SerializationTests.cpp
)

target_include_directories(commonTests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(commonTests
    common
    gmock_main
    Qt::Core
)

add_test(
    NAME commonTests
    COMMAND commonTests
)

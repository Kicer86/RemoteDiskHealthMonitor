
find_package(Qt6 REQUIRED COMPONENTS Core Network Test)

add_executable(monitorTests
    AgentsListTests.cpp
    IAgentsStatusProviderMock.hpp
    ${PROJECT_SOURCE_DIR}/src/monitor/AgentsList.cpp
    ${PROJECT_SOURCE_DIR}/src/monitor/AgentsList.hpp
    ${PROJECT_SOURCE_DIR}/src/monitor/AgentInformation.cpp
    ${PROJECT_SOURCE_DIR}/src/monitor/IAgentsStatusProvider.hpp
)

target_include_directories(monitorTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/monitor
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/external/nlohmann-json/single_include
)

target_link_libraries(monitorTests
    common
    gmock_main

    Qt::Core
    Qt::Network
    Qt::Test
)

set_target_properties(monitorTests PROPERTIES AUTOMOC TRUE)

add_test(
    NAME monitorTests
    COMMAND monitorTests
)

add_executable(monitorIntegrationTests
    AgentsStatusProviderIntegrationTests.cpp
    ${PROJECT_SOURCE_DIR}/src/monitor/AgentsStatusProvider.cpp
    ${PROJECT_SOURCE_DIR}/src/monitor/AgentsStatusProvider.hpp
    ${PROJECT_SOURCE_DIR}/src/monitor/AgentInformation.cpp
    ${PROJECT_SOURCE_DIR}/src/monitor/AgentInformation.hpp
    ${PROJECT_SOURCE_DIR}/src/monitor/IAgentsStatusProvider.hpp
)

target_include_directories(monitorIntegrationTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/monitor
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/external/nlohmann-json/single_include
)

target_include_directories(monitorIntegrationTests
    SYSTEM PRIVATE
        ${PROJECT_SOURCE_DIR}/external/cpp-httplib
)

target_link_libraries(monitorIntegrationTests
    common
    cpp_restapi
    gmock
    gtest

    Qt::Core
    Qt::Network
    Qt::Test
)

set_target_properties(monitorIntegrationTests PROPERTIES AUTOMOC TRUE)

add_test(
    NAME monitorIntegrationTests
    COMMAND monitorIntegrationTests
)

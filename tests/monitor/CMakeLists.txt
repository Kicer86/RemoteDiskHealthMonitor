
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

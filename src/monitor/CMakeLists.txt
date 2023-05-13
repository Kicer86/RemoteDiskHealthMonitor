
find_package(Qt6 REQUIRED COMPONENTS Quick Gui Network RemoteObjects)

qt6_add_resources(QML_RESOURCES
    Qml/resources.qrc
    images/images.qrc
)

add_executable(monitor
    AgentsList.cpp
    AgentsList.hpp
    AgentInformation.cpp
    AgentInformation.hpp
    AgentsExplorer.cpp
    AgentsExplorer.hpp
    AgentsStatusProvider.cpp
    AgentsStatusProvider.hpp
    Configuration.cpp
    Configuration.hpp
    IAgentsStatusProvider.hpp
    main.cpp
    ManualAgentsValidator.cpp
    ManualAgentsValidator.hpp
    ${QML_RESOURCES}
)

qt_add_repc_replicas(monitor ${PROJECT_SOURCE_DIR}/src/common/AgentStatus.rep)

target_link_libraries(monitor
    PRIVATE
        common

        Qt::Network
        Qt::Quick
        Qt::Gui
        Qt::Network
        Qt::RemoteObjects
        QtZeroConf
)

target_include_directories(monitor
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
)

set_target_properties(monitor PROPERTIES AUTOMOC TRUE)

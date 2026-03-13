
find_package(Qt6 REQUIRED COMPONENTS Quick Gui Network QuickControls2)

qt_add_executable(monitor
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
    HealthEnumQml.hpp
    IAgentsStatusProvider.hpp
    main.cpp
    ManualAgentsValidator.cpp
    ManualAgentsValidator.hpp
)

set_target_properties(monitor PROPERTIES AUTOMOC TRUE)

qt_add_qml_module(monitor
    URI RDHM.Monitor
    VERSION 1.0
    QML_FILES
        Qml/Main.qml
        Qml/AgentCard.qml
        Qml/AgentDetailPanel.qml
        Qml/SmartDataTable.qml
        Qml/AddAgentDialog.qml
        Qml/StatusBadge.qml
    RESOURCES
        images/cancel.svg
        images/check.svg
        images/close.svg
        images/hard-disk.svg
        images/hard-disk2.svg
        images/ok.svg
        images/question-mark.svg
        images/trash.svg
        images/warning.svg
    NO_RESOURCE_TARGET_PATH
)

target_link_libraries(monitor
    PRIVATE
        common

        Qt::Network
        Qt::Quick
        Qt::Gui
        Qt::QuickControls2
        QtZeroConf
)

target_include_directories(monitor
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/external/nlohmann-json/single_include
)

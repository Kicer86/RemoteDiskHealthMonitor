
add_executable(agentTests
    DiscStatusCalculatorTests.cpp
    DiskTests.cpp
    DmesgParserTests.cpp
    LinuxDiskCollectorTests.cpp
    LsblkOutputParserTests.cpp
    OutputParsersUtilsTests.cpp
    SmartCtlOutputParserTests.cpp
    SmartHealthAnalyzerTests.cpp
    VendorProfileTests.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/Disk.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/DiscStatusCalculator.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/OutputParsersUtils.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/SmartHealthAnalyzer.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/linux/DmesgParser.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/linux/LinuxDiskCollector.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/linux/LsblkOutputParser.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/linux/SmartCtlOutputParser.cpp
    IPartitionsManagerMock.h
    IProbeMock.h
    ISmartReaderMock.h
    DiskMock.h)

target_include_directories(agentTests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/agent
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(agentTests
    common
    gmock_main
)

add_test(
    NAME agentTests
    COMMAND agentTests
)


ADD_EXECUTABLE( agent
    main.cpp
    IProbe.h
    ISmartReader.h
    SmartReader.h
    HttpServer.h
    HttpServer.cpp
    MdnsNetwork.h
    MdnsNetwork.cpp
    MdnsPublisher.h
    MdnsPublisher.cpp
    OutputParsersUtils.h
    OutputParsersUtils.cpp

    DiscStatusCalculator.h
    DiscStatusCalculator.cpp
    IDiskCollector.h
    Disk.h
    Disk.cpp
    SystemUtilitiesFactory.h
    VendorProfile.h
    SmartHealthAnalyzer.h
    SmartHealthAnalyzer.cpp
)

target_include_directories(agent
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/external/cpp-httplib
        ${PROJECT_SOURCE_DIR}/external/nlohmann-json/single_include
        ${PROJECT_SOURCE_DIR}/external/mdns
)

target_link_libraries(agent
    PRIVATE
        common
)

if(WIN32)
    find_library(WBEMUUID_LIBRARY wbemuuid
        PATHS "C:/Program Files (x86)/Windows Kits/10/Lib/*/um/x64"
        REQUIRED
    )

    set_target_properties(agent PROPERTIES LINK_FLAGS " /MANIFESTUAC:\"level='requireAdministrator'\"")

    target_sources(agent
        PRIVATE
            windows/WinGeneralAnalyzer.h
            windows/WinGeneralAnalyzer.cpp
            windows/WindowsSmartReader.cpp
            windows/WMICommunication.h
            windows/WMICommunication.cpp
            windows/CMDCommunication.h
            windows/CMDCommunication.cpp
            windows/WinDiskCollector.h
            windows/WinDiskCollector.cpp
            windows/WinSystemUtilitiesFactory.cpp
    )

    target_link_libraries(agent
        PRIVATE
            ${WBEMUUID_LIBRARY}
            ws2_32
            iphlpapi
    )
elseif(UNIX AND NOT APPLE)
    target_sources(agent
        PRIVATE
            linux/DmesgParser.cpp
            linux/DmesgParser.h
            linux/IPartitionsManager.h
            linux/LinuxSmartReader.cpp
            linux/LinGeneralAnalyzer.h
            linux/LinGeneralAnalyzer.cpp
            linux/LinSystemUtilitiesFactory.cpp
            linux/LinuxDiskCollector.cpp
            linux/LinuxDiskCollector.h
            linux/LsblkOutputParser.cpp
            linux/LsblkOutputParser.h
            linux/SmartCtlOutputParser.cpp
            linux/SmartCtlOutputParser.h
    )

    target_link_libraries(agent
        PRIVATE
            pthread
    )
elseif(APPLE)
    target_sources(agent
        PRIVATE
            linux/DmesgParser.cpp
            linux/DmesgParser.h
            linux/IPartitionsManager.h
            linux/LinuxSmartReader.cpp
            linux/LinGeneralAnalyzer.h
            linux/LinGeneralAnalyzer.cpp
            linux/LinSystemUtilitiesFactory.cpp
            linux/LinuxDiskCollector.cpp
            linux/LinuxDiskCollector.h
            linux/LsblkOutputParser.cpp
            linux/LsblkOutputParser.h
            linux/SmartCtlOutputParser.cpp
            linux/SmartCtlOutputParser.h
    )

    target_link_libraries(agent
        PRIVATE
            pthread
    )
else()
    message(FATAL_ERROR "Unsupported system")
endif()

set_target_properties(agent PROPERTIES OUTPUT_NAME rdhm-agent)

include(GNUInstallDirs)
install(TARGETS agent RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})

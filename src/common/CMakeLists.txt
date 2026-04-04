
add_library(common OBJECT
    constants.hpp
    GeneralHealth.h
    ProbeStatus.h
    ProtocolVersion.h
    SmartData.cpp
    SmartData.h
    GeneralHealth.cpp
    DiskSummary.h
    DiskInfo.h
    DiskInfo.cpp
    JsonSerialize.h)

target_include_directories(common
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/external/nlohmann-json/single_include
)

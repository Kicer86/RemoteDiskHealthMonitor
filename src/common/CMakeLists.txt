
find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(common OBJECT
    constants.hpp
    GeneralHealth.h
    ProbeStatus.h
    ProbeStatusSerialize.h
    ProtocolVersion.h
    OutputParsersUtils.cpp
    OutputParsersUtils.h
    GeneralHealth.cpp
    DiskInfo.h
    DiskInfoSerialize.h
    DiskInfo.cpp)

target_include_directories(common
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(common
    PUBLIC
        Qt::Core
)

set_target_properties(common PROPERTIES AUTOMOC TRUE)

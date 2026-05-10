
cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.16)
project(RemoteDiscHealthMonitor VERSION 0.2.0)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

option(BUILD_AGENT   "Build the agent component"   ON)
option(BUILD_MONITOR "Build the monitor component" ON)

add_custom_target(sync-package-version
    COMMAND ${CMAKE_COMMAND} -P ${PROJECT_SOURCE_DIR}/packaging/sync-package-version.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Synchronizing package metadata with project version ${PROJECT_VERSION}"
)

add_custom_target(verify-package-version
    COMMAND ${CMAKE_COMMAND} -DVERIFY_ONLY=ON -P ${PROJECT_SOURCE_DIR}/packaging/sync-package-version.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Verifying package metadata against project version ${PROJECT_VERSION}"
)

if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)      # when msvc, build gtest with shared crt library
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(BUILD_SHARED_LIBS OFF)
else()
    set(BUILD_SHARED_LIBS ON)
endif()

enable_testing()

#external projects
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(external/googletest EXCLUDE_FROM_ALL)

if(BUILD_MONITOR)
    add_subdirectory(external/QtZeroConf EXCLUDE_FROM_ALL)
    set_target_properties(QtZeroConf PROPERTIES
        EXCLUDE_FROM_ALL FALSE
        PUBLIC_HEADER ""
    )

    install(TARGETS QtZeroConf
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    set(CppRestAPI_QtBackend      ON  CACHE BOOL "" FORCE)
    set(CppRestAPI_GitHub         OFF CACHE BOOL "" FORCE)
    set(CppRestAPI_JsonPagination OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/cpp_restapi EXCLUDE_FROM_ALL)
endif()
#

add_subdirectory(src/common)

if(BUILD_AGENT)
    add_subdirectory(src/agent)
endif()

if(BUILD_MONITOR)
    add_subdirectory(src/monitor)
endif()

add_subdirectory(tests)

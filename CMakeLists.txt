
cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.16)
project(RemoteDiscHealthMonitor VERSION 0.2.0)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_STANDARD 23)

option(BUILD_AGENT   "Build the agent component"   ON)
option(BUILD_MONITOR "Build the monitor component" ON)

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
    add_subdirectory(external/QtZeroConf)

    # jsoncpp is a transitive dependency of cpp_restapi. Build it from the bundled
    # submodule instead of relying on a system-provided package.
    set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)
    set(JSONCPP_WITH_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE BOOL "" FORCE)
    set(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE BOOL "" FORCE)
    set(JSONCPP_WITH_INSTALL OFF CACHE BOOL "" FORCE)
    set(BUILD_STATIC_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/jsoncpp EXCLUDE_FROM_ALL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/external/jsoncpp/jsoncpp-namespaced-targets.cmake)

    # Short-circuit cpp_restapi's find_package(jsoncpp CONFIG) so it picks up
    # the in-tree JsonCpp::JsonCpp target defined above instead of searching
    # for a system install.
    set(_jsoncpp_stub_dir "${CMAKE_BINARY_DIR}/jsoncpp-stub")
    file(WRITE "${_jsoncpp_stub_dir}/jsoncppConfig.cmake"
         "set(jsoncpp_FOUND TRUE)\n")
    set(jsoncpp_DIR "${_jsoncpp_stub_dir}" CACHE PATH "" FORCE)

    set(CppRestAPI_QtBackend ON CACHE BOOL "" FORCE)
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

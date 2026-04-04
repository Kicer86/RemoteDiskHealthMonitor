
if(BUILD_AGENT)
    add_subdirectory(agent)
endif()

add_subdirectory(common)

if(BUILD_MONITOR)
    add_subdirectory(monitor)
endif()

set(_test_deps commonTests)

if(BUILD_AGENT)
    list(APPEND _test_deps agentTests)
endif()

if(BUILD_MONITOR)
    list(APPEND _test_deps monitorTests)
endif()

add_custom_target(RunUnitTests ALL
    COMMAND ctest -C $<CONFIG>
    DEPENDS ${_test_deps}
)

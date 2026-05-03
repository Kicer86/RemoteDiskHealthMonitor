cmake_minimum_required(VERSION 3.16)

set(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

file(READ "${PROJECT_ROOT}/CMakeLists.txt" ROOT_CMAKELISTS)
string(REGEX MATCH
    "project\\([^)]*VERSION[ \t\r\n]+([0-9]+\\.[0-9]+\\.[0-9]+)[^)]*\\)"
    PROJECT_DECLARATION
    "${ROOT_CMAKELISTS}"
)

if(NOT PROJECT_DECLARATION)
    message(FATAL_ERROR "Could not find project(... VERSION x.y.z) in ${PROJECT_ROOT}/CMakeLists.txt")
endif()

set(PROJECT_PACKAGE_VERSION "${CMAKE_MATCH_1}")
set(PACKAGE_RELEASE "1")

if(PRINT_VERSION)
    execute_process(COMMAND "${CMAKE_COMMAND}" -E echo "${PROJECT_PACKAGE_VERSION}")
    return()
endif()

if(VERIFY_ONLY)
    set(SYNC_VERIFY_ONLY TRUE)
else()
    set(SYNC_VERIFY_ONLY FALSE)
endif()

set(SYNC_MISMATCHES "")

function(_record_mismatch path)
    list(APPEND SYNC_MISMATCHES "${path}")
    set(SYNC_MISMATCHES "${SYNC_MISMATCHES}" PARENT_SCOPE)
endfunction()

function(_write_if_changed path content)
    if(EXISTS "${path}")
        file(READ "${path}" old_content)
    else()
        set(old_content "")
    endif()

    if(NOT old_content STREQUAL content)
        if(SYNC_VERIFY_ONLY)
            _record_mismatch("${path}")
        else()
            file(WRITE "${path}" "${content}")
            message(STATUS "Updated ${path}")
        endif()
    endif()
endfunction()

function(_replace_in_file path pattern replacement)
    file(READ "${path}" old_content)
    string(REGEX MATCH "${pattern}" matched_content "${old_content}")
    if(NOT matched_content)
        if(SYNC_VERIFY_ONLY)
            _record_mismatch("${path}")
            return()
        endif()
        message(FATAL_ERROR "Could not update ${path}; pattern did not match: ${pattern}")
    endif()

    string(REGEX REPLACE "${pattern}" "${replacement}" new_content "${old_content}")

    if(old_content STREQUAL new_content)
        return()
    else()
        _write_if_changed("${path}" "${new_content}")
    endif()
endfunction()

function(_sync_debian_changelog component)
    set(changelog_path "${PROJECT_ROOT}/packaging/deb/${component}/debian/changelog")
    set(control_path "${PROJECT_ROOT}/packaging/deb/${component}/debian/control")
    set(expected_version "${PROJECT_PACKAGE_VERSION}-${PACKAGE_RELEASE}")

    file(READ "${changelog_path}" changelog)
    string(REGEX MATCH "^${component} \\(([^)]+)\\)" first_entry "${changelog}")

    if(NOT first_entry)
        if(SYNC_VERIFY_ONLY)
            _record_mismatch("${changelog_path}")
            return()
        endif()
        message(FATAL_ERROR "Could not parse first Debian changelog entry in ${changelog_path}")
    endif()

    if(CMAKE_MATCH_1 STREQUAL expected_version)
        return()
    endif()

    if(SYNC_VERIFY_ONLY)
        _record_mismatch("${changelog_path}")
        return()
    endif()

    file(READ "${control_path}" control)
    string(REGEX MATCH "(^|\n)Maintainer:[ \t]*([^\n]+)" maintainer_match "${control}")
    if(maintainer_match)
        set(maintainer "${CMAKE_MATCH_2}")
    else()
        set(maintainer "Michał Walenciak <michalwalenciak@gmail.com>")
    endif()

    string(TIMESTAMP debian_date "%a, %d %b %Y %H:%M:%S +0000" UTC)
    set(new_entry
"${component} (${expected_version}) unstable; urgency=medium

  * Release version ${PROJECT_PACKAGE_VERSION}.

 -- ${maintainer}  ${debian_date}

")
    _write_if_changed("${changelog_path}" "${new_entry}${changelog}")
endfunction()

function(_sync_rpm_changelog component maintainer)
    set(spec_path "${PROJECT_ROOT}/packaging/rpm/${component}.spec")
    set(expected_version "${PROJECT_PACKAGE_VERSION}-${PACKAGE_RELEASE}")

    file(READ "${spec_path}" spec)
    string(REGEX MATCH "\\* [^\n]+ - ([0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+)" first_entry "${spec}")

    if(first_entry AND CMAKE_MATCH_1 STREQUAL expected_version)
        return()
    endif()

    if(SYNC_VERIFY_ONLY)
        _record_mismatch("${spec_path}")
        return()
    endif()

    string(TIMESTAMP rpm_date "%a %b %d %Y" UTC)
    set(new_changelog
"%changelog
* ${rpm_date} ${maintainer} - ${expected_version}
- Release version ${PROJECT_PACKAGE_VERSION}
")
    string(REGEX REPLACE "%changelog\n" "${new_changelog}" updated_spec "${spec}")
    _write_if_changed("${spec_path}" "${updated_spec}")
endfunction()

_replace_in_file(
    "${PROJECT_ROOT}/packaging/arch/rdhm-agent/PKGBUILD"
    "pkgver=[^\n]+"
    "pkgver=${PROJECT_PACKAGE_VERSION}"
)
_replace_in_file(
    "${PROJECT_ROOT}/packaging/arch/rdhm-monitor/PKGBUILD"
    "pkgver=[^\n]+"
    "pkgver=${PROJECT_PACKAGE_VERSION}"
)

_replace_in_file(
    "${PROJECT_ROOT}/packaging/rpm/rdhm-agent.spec"
    "Version:[ \t]+[^\n]+"
    "Version:        ${PROJECT_PACKAGE_VERSION}"
)
_replace_in_file(
    "${PROJECT_ROOT}/packaging/rpm/rdhm-monitor.spec"
    "Version:[ \t]+[^\n]+"
    "Version:        ${PROJECT_PACKAGE_VERSION}"
)

_replace_in_file(
    "${PROJECT_ROOT}/packaging/deb/rdhm-agent/build.sh"
    "PKG_DIR=\"\\$BUILD_DIR/rdhm-agent-[^\"]+\""
    "PKG_DIR=\"$BUILD_DIR/rdhm-agent-${PROJECT_PACKAGE_VERSION}\""
)
_replace_in_file(
    "${PROJECT_ROOT}/packaging/deb/rdhm-monitor/build.sh"
    "PKG_DIR=\"\\$BUILD_DIR/rdhm-monitor-[^\"]+\""
    "PKG_DIR=\"$BUILD_DIR/rdhm-monitor-${PROJECT_PACKAGE_VERSION}\""
)

_sync_debian_changelog("rdhm-agent")
_sync_debian_changelog("rdhm-monitor")

_sync_rpm_changelog("rdhm-agent" "Michał Walenciak <michalwalenciak@gmail.com>")
_sync_rpm_changelog("rdhm-monitor" "Michał Walenciak <michalwalenciak@gmail.com>")

if(SYNC_MISMATCHES)
    list(JOIN SYNC_MISMATCHES "\n  " mismatch_text)
    message(FATAL_ERROR "Package metadata is out of sync with project version ${PROJECT_PACKAGE_VERSION}:\n  ${mismatch_text}\nRun: cmake -P packaging/sync-package-version.cmake")
endif()

message(STATUS "Package metadata is synchronized with version ${PROJECT_PACKAGE_VERSION}")

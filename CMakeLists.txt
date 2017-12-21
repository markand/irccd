#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

#
# Where to start.
# -------------------------------------------------------------------
#
# If you plan to modify the build system there are several places to look to do
# what you want.
#
# cmake/IrccdOptions.cmake  - User definable options.
# cmake/IrccdPackage.cmake  - Package creation.
# cmake/IrccdSystem.cmake   - Contains some platforms checks and compile flags.
# cmake/IrccdVersion.cmake  - Defines the Irccd version and its plugins.
# cmake/check               - Platform checks in separate files.
# cmake/function            - Custom CMake functions.
# cmake/installer           - Some files for the QtIFW installer.
# cmake/internal            - Some internal files (e.g. the sysconfig.h)
# cmake/packages            - Additional find_package modules.
#
# Build system is then processed in different directories:
#
# contrib                   - User contributions.
# doc                       - The documentation process.
# extern                    - External libraries.
# lib                       - The irccd library
# irccd                     - The irccd executable.
# irccdctl                  - The irccdctl utility.
# plugins                   - Official irccd plugins.
# tests                     - The unit tests.
# win32                     - Additional files for Windows platform.
#

cmake_minimum_required(VERSION 3.3)
project(irccd CXX)

# Helper to set global internal variables.
function(setg var value)
    set("${var}" "${value}" CACHE INTERNAL "")
endfunction ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${irccd_SOURCE_DIR}/cmake/packages)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CMakeParseArguments)

include(cmake/function/IrccdBuildHtml.cmake)
include(cmake/function/IrccdDefineExecutable.cmake)
include(cmake/function/IrccdDefineLibrary.cmake)
include(cmake/function/IrccdDefineMan.cmake)
include(cmake/function/IrccdDefinePlugin.cmake)
include(cmake/function/IrccdDefineTest.cmake)
include(cmake/function/IrccdIndentMessage.cmake)

include(cmake/check/PutTime.cmake)

include(cmake/IrccdVersion.cmake)
include(cmake/IrccdOptions.cmake)
include(cmake/IrccdSystem.cmake)

find_package(Threads REQUIRED)

add_subdirectory(extern/json)
add_subdirectory(doc)
add_subdirectory(libcommon)
add_subdirectory(libirccd)
add_subdirectory(libirccdctl)

if (HAVE_JS)
    add_subdirectory(libirccd-js)
endif ()

add_subdirectory(irccd)
add_subdirectory(irccdctl)
add_subdirectory(irccd-test)
add_subdirectory(contrib)

if (HAVE_JS)
    add_subdirectory(plugins)
endif ()

# Platform specific.
if (WIN32)
    add_subdirectory(win32)
endif ()

# Tests.
if (WITH_TESTS)
    include(CTest)
    add_subdirectory(libirccd-test)
    add_subdirectory(tests)
endif ()

message("Compiling with the following flags:")
message("    General flags:    ${CMAKE_CXX_FLAGS}")
message("    Debug flags:      ${CMAKE_CXX_FLAGS_DEBUG}")
message("    Release flags:    ${CMAKE_CXX_FLAGS_RELEASE}")
message("")

message("Compiling irccd with following options:")
message("    Libedit:          ${WITH_LIBEDIT_MSG}")
message("    OpenSSL:          ${WITH_SSL_MSG}")
message("    JS:               ${WITH_JS_MSG}")
message("    Tests:            ${WITH_TESTS_MSG}")
message("    User docs:        ${WITH_HTML_MSG}")
message("    Doxygen:          ${WITH_DOXYGEN_MSG}")
message("    Package:          ${IRCCD_PACKAGE_MSG}")
message("    Vera:             ${WITH_VERA_MSG}")
message("")

message("Installing plugins:")
foreach (plugin ${IRCCD_PLUGINS})
    string(TOUPPER ${plugin} name)
    irccd_indent_message("    ${plugin}: " "${WITH_PLUGIN_${name}_MSG}" 22)
endforeach ()
message("")

if (WIN32)
    message("Installing these DLLs:")
    foreach (name ${IRCCD_DLLS})
        irccd_indent_message("    ${name}: " "${${name}}" 30)
    endforeach ()
    message("")

    if (IRCCD_DLLS_NOT_FOUND)
        message("The following DLLs were not found:")
        foreach (name ${IRCCD_DLLS_NOT_FOUND})
            message("    ${name}")
        endforeach ()
        message("")
    endif ()
endif ()

include(cmake/IrccdPackage.cmake)
include(CPack)

# Metadata files
add_custom_target(
    metadata
    SOURCES
        ${CMAKE_SOURCE_DIR}/CHANGES.md
        ${CMAKE_SOURCE_DIR}/CONTRIBUTE.md
        ${CMAKE_SOURCE_DIR}/CREDITS.md
        ${CMAKE_SOURCE_DIR}/INSTALL.md
        ${CMAKE_SOURCE_DIR}/MIGRATING.md
        ${CMAKE_SOURCE_DIR}/README.md
        ${CMAKE_SOURCE_DIR}/STYLE.md
)

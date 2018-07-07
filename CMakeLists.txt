#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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
# doc                       - The documentation process.
# extern                    - External libraries.
# libirccdctl               - The irccdctl library.
# libirccd-js               - Javascript bindings library.
# libirccd-test             - Helpers for unit tests.
# libirccd                  - The irccd library.
# irccd                     - The irccd executable.
# irccdctl                  - The irccdctl utility.
# plugins                   - Official irccd plugins.
# systemd                   - Unit file for systemd.
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
set(CMAKE_POSITION_INDEPENDENT_CODE On)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS On)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

include(CMakeParseArguments)
include(GNUInstallDirs)

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
add_subdirectory(libirccd-test)

if (IRCCD_HAVE_JS)
    add_subdirectory(libirccd-js)
endif ()

add_subdirectory(irccd)
add_subdirectory(irccdctl)
add_subdirectory(irccd-test)
add_subdirectory(systemd)

if (IRCCD_HAVE_JS)
    add_subdirectory(plugins)
endif ()

# Platform specific.
if (WIN32)
    add_subdirectory(win32)
endif ()

# Tests.
if (IRCCD_WITH_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif ()

message("Compiling with the following flags:")
message("    General flags:    ${CMAKE_CXX_FLAGS}")
message("    Debug flags:      ${CMAKE_CXX_FLAGS_DEBUG}")
message("    Release flags:    ${CMAKE_CXX_FLAGS_RELEASE}")
message("")

message("Compiling irccd with following options:")
message("    Libedit:          ${IRCCD_WITH_LIBEDIT_MSG}")
message("    OpenSSL:          ${IRCCD_WITH_SSL_MSG}")
message("    Javascript:       ${IRCCD_WITH_JS_MSG}")
message("    Tests:            ${IRCCD_WITH_TESTS_MSG}")
message("    User docs:        ${IRCCD_WITH_HTML_MSG}")
message("    Doxygen:          ${IRCCD_WITH_DOXYGEN_MSG}")
message("    Package:          ${IRCCD_PACKAGE_MSG}")
message("    Vera:             ${IRCCD_WITH_VERA_MSG}")
message("")

message("Installing plugins:")
foreach (plugin ${IRCCD_PLUGINS})
    string(TOUPPER ${plugin} name)
    irccd_indent_message("    ${plugin}: " "${IRCCD_WITH_PLUGIN_${name}_MSG}" 22)
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

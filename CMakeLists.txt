#
# CMakeLists.txt -- CMake build for irccd
#
# Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

cmake_minimum_required(VERSION 3.12)
project(irccd)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set(IRCCD_VERSION_MAJOR 4)
set(IRCCD_VERSION_MINOR 0)
set(IRCCD_VERSION_PATCH 0)
set(IRCCD_VERSION "${IRCCD_VERSION_MAJOR}.${IRCCD_VERSION_MINOR}.${IRCCD_VERSION_PATCH}")

option(IRCCD_WITH_JS "Enable Javascript" On)
option(IRCCD_WITH_SSL "Enable SSL support" On)
option(IRCCD_WITH_IRCCD "Enable irccd daemon" On)
option(IRCCD_WITH_IRCCDCTL "Enable irccdctl utility" On)
option(IRCCD_WITH_TESTS "Enable unit tests" Off)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_C_FLAGS "-Wall -Wextra ${CMAKE_C_FLAGS}")
endif ()

include(GNUInstallDirs)

include(cmake/IrccdDefinePlugin.cmake)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

if (IRCCD_WITH_SSL)
	find_package(OpenSSL REQUIRED)
endif ()

add_subdirectory(extern/libcompat)
add_subdirectory(extern/libketopt)

if (IRCCD_WITH_JS)
	add_subdirectory(extern/libduktape)
endif ()

add_subdirectory(lib)
add_subdirectory(irccd)
add_subdirectory(irccdctl)
add_subdirectory(plugins)
add_subdirectory(man)

if (IRCCD_WITH_TESTS)
	enable_testing()
	add_subdirectory(extern/libgreatest)
	add_subdirectory(tests)
endif ()

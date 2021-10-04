#
# CMakeLists.txt -- CMake build system for irccd
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

cmake_minimum_required(VERSION 2.20)
cmake_policy(SET CMP0048 NEW)
project(irccd
	LANGUAGES C
	DESCRIPTION "IRC Client Daemon"
	HOMEPAGE_URL "http://projects.malikania.fr/irccd"
	VERSION 4.0.0
)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED On)
set(CMAKE_C_EXTENSIONS Off)
set(CMAKE_POSITION_INDEPENDENT_CODE On)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic ${CMAKE_C_FLAGS}")
endif ()

option(IRCCD_WITH_EXAMPLES "Enable example files" On)
option(IRCCD_WITH_JS "Enable Javascript" On)
option(IRCCD_WITH_MAN "Enable manual pages" On)
option(IRCCD_WITH_SSL "Enable SSL (requires OpenSSL)" On)
option(IRCCD_WITH_TESTS "Enable unit tests" On)

include(GNUInstallDirs)

include(cmake/IrccdDefinePlugin.cmake)

if (IRCCD_WITH_SSL)
	find_package(OpenSSL REQUIRED)
endif ()

if (IRCCD_WITH_JS)
	add_subdirectory(extern/libduktape)
endif ()

add_subdirectory(extern/libutlist)
add_subdirectory(extern/libketopt)

add_subdirectory(lib)
add_subdirectory(irccd)
add_subdirectory(irccdctl)
add_subdirectory(plugins)

if (IRCCD_WITH_EXAMPLES)
	add_subdirectory(examples)
endif ()
if (IRCCD_WITH_MAN)
	add_subdirectory(man)
endif ()

if (IRCCD_WITH_TESTS)
	enable_testing()
	add_subdirectory(extern/libgreatest)
	add_subdirectory(tests)
endif ()

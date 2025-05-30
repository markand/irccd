#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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
# Overview of projects targets
# ============================
#
# Irccd is a host program that loads C native plugins, to do so it exports the
# public API by its own and plugins must not link to the library code itself.
# But for unit testing the project we don't want to compile over and over the
# same files that *must* link against the library code so we split:
#
# - libirccd (lib/, static library): contains the public API for C plugins.
# - irccd-static (irccd/, object code): contains all irccd(1) code *without*
#   the main entry point. This code isn't publicly exposed to the plugins.
# - irccd (irccd/, executable): contains main and config parser code.
# - irccdctl (irccdctl, executable): contain irccdctl(1) utility.
#

cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0048 NEW)
project(irccd
	LANGUAGES C
	DESCRIPTION "IRC Client Daemon"
	HOMEPAGE_URL "http://projects.malikania.fr/irccd"
	VERSION 5.0.0
)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED On)
set(CMAKE_C_EXTENSIONS On)
set(CMAKE_POSITION_INDEPENDENT_CODE On)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic ${CMAKE_C_FLAGS}")
endif ()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(IRCCD_WITH_EXAMPLES "Enable example files" On)
option(IRCCD_WITH_HTTP "Enable HTTP support (requires libcurl)" On)
option(IRCCD_WITH_JS "Enable Javascript" On)
option(IRCCD_WITH_MAN "Enable manual pages" On)
option(IRCCD_WITH_PKGCONFIG "Enable installation of pkg-config file" On)
option(IRCCD_WITH_SSL "Enable SSL (requires OpenSSL)" On)
option(IRCCD_WITH_TESTS "Enable unit tests" On)

set(IRCCD_MAN_DATE "April 23, 2025")
set(IRCCD_WITH_UID "irccd" CACHE STRING "Default uid to run irccd as")
set(IRCCD_WITH_GID "irccd" CACHE STRING "Default gid to run irccd as")
set(IRCCD_WITH_PKGCONFIGDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
	CACHE STRING "Directory for pkg-config files")

# Check presence of POSIX m library
find_library(M_LIBRARY m libm)

# The libev library.
find_package(Ev REQUIRED)

include(cmake/IrccdDefinePlugin.cmake)

if (IRCCD_WITH_SSL)
	find_package(OpenSSL REQUIRED)
endif ()

if (IRCCD_WITH_JS)
	add_subdirectory(extern/libduktape)
endif ()

add_subdirectory(extern/libutlist)

add_subdirectory(cmake)
add_subdirectory(lib)
add_subdirectory(irccd)
add_subdirectory(irccdctl)
add_subdirectory(plugins)
add_subdirectory(systemd)

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

# CMake packages.
write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/irccd-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_file(
	${PROJECT_SOURCE_DIR}/cmake/irccd-config.cmake.in
	${PROJECT_BINARY_DIR}/irccd-config.cmake
	@ONLY
)

install(
	FILES
		${PROJECT_BINARY_DIR}/irccd-config.cmake
		${PROJECT_BINARY_DIR}/irccd-config-version.cmake
		${PROJECT_SOURCE_DIR}/cmake/IrccdDefinePlugin.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/irccd
)

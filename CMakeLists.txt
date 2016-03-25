#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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
# Where to start
# ---------------------------------------------------------
#
# If you plan to modify the build system there are several places to look to do what you want.
#
# cmake/IrccdOptions.cmake	- User definable options.
# cmake/IrccdPackage.cmake	- Package creation.
# cmake/IrccdSystem.cmake	- Contains some platforms checks and compile flags.
# cmake/IrccdVersion.cmake	- Defines the Irccd version and its plugins.
# cmake/check			- Platform checks in separate files.
# cmake/function		- Custom functions.
# cmake/installer		- Some files for the QtIFW installer.
# cmake/internal		- Some internal files (e.g. the irccd-config.h)
# cmake/packages		- Additional find_package modules.
#
# Build system is then processed in different directories:
#
# contrib			- User contributions not maintained by irccd authors.
# doc				- The documentation process.
# extern			- External libraries.
# lib				- The irccd library
# irccd				- The irccd executable.
# irccdctl			- The irccdctl utility.
# plugins			- Official irccd plugins.
# tests				- The unit tests.
# win32				- Additional files for Windows platform.
#

cmake_minimum_required(VERSION 3.0)
project(irccd)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${irccd_SOURCE_DIR}/cmake/packages)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

include(CMakeParseArguments)

include(cmake/function/IrccdDefineExecutable.cmake)
include(cmake/function/IrccdDefineHtml.cmake)
include(cmake/function/IrccdDefineLibrary.cmake)
include(cmake/function/IrccdDefineMan.cmake)
include(cmake/function/IrccdDefinePlugin.cmake)
include(cmake/function/IrccdDefineTest.cmake)

include(cmake/check/PutTime.cmake)

include(cmake/IrccdVersion.cmake)
include(cmake/IrccdOptions.cmake)
include(cmake/IrccdSystem.cmake)

add_subdirectory(extern/libircclient)
add_subdirectory(extern/jansson)
add_subdirectory(doc)
add_subdirectory(lib)
add_subdirectory(irccd)
add_subdirectory(irccdctl)
add_subdirectory(contrib)

if (WITH_JS)
	add_subdirectory(plugins)
endif ()

# Platform specific.
if (WIN32)
	add_subdirectory(win32)
endif ()

# Tests.
include(CTest)
add_subdirectory(tests)

message("Compiling with the following flags:")
message("    General flags:    ${CMAKE_CXX_FLAGS}")
message("    Debug flags:      ${CMAKE_CXX_FLAGS_DEBUG}")
message("    Release flags:    ${CMAKE_CXX_FLAGS_RELEASE}")
message("")

message("Compiling irccd with following options:")
message("    OpenSSL:          ${WITH_SSL_MSG}")
message("    JS:               ${WITH_JS_MSG}")
message("    Tests:            ${WITH_TESTS_MSG}")
message("    User docs:        ${WITH_HTML_MSG}")
message("    Doxygen:          ${WITH_DOXYGEN_MSG}")
message("    Package:          ${IRCCD_PACKAGE_MSG}")
message("")

message("Installing plugins:")
foreach (plugin ${IRCCD_PLUGINS})
	string(TOUPPER ${plugin} name)
	string(LENGTH ${plugin} length)
	set(str "    ${plugin}:")

	#
	# Build a string to indent the output correctly because tabs do not work well in all windows
	# (e.g. CMake's GUI, QtCreator...)
	#
	while (${length} LESS 17)
		math(EXPR length "${length} + 1")
		set(str "${str} ")
	endwhile ()

	message("${str}${WITH_PLUGIN_${name}_MSG}")
endforeach ()

message("")

# CPack (only for package_source, package_ifw is home made).
#include(cmake/IrccdPackage.cmake)
#include(CPack)

# Meta release target.
#if (IRCCD_PACKAGE)
#	add_custom_target(
#		release
#		COMMENT "Releasing irccd ${IRCCD_VERSION}"
#		COMMAND
#			${CMAKE_MAKE_PROGRAM} package_source
#	)
#
#	add_dependencies(release package_ifw)
#endif ()

# Show warnings about non-relocatable irccd.
if (NOT IRCCD_RELOCATABLE)
	message("Note: irccd is not relocatable, the following paths are absolute:")
	message("")

	foreach (path ${IRCCD_ABSOLUTE_PATHS})
		message("    ${path} is set to ${${path}}")
	endforeach ()

	message("")
	message("You can still use irccd, but you won't be able to move the application to a")
	message("different directory.")
	message("")
	message("This may be an issue with some packages manager which allows relocating")
	message("application in different directories.")
	message("")
	message("Set to relative paths if this is a concern.")
	message("")
endif ()

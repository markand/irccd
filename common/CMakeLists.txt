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

project(common)

set(
	SOURCES
	directory.cpp
	directory.h
	elapsed-timer.cpp
	elapsed-timer.h
	filesystem.cpp
	filesystem.h
	ini.cpp
	ini.h
	json.cpp
	json.h
	logger.cpp
	logger.h
	options.cpp
	options.h
	path.cpp
	path.h
	signals.h
	sockets.cpp
	sockets.h
	system.h
	system.cpp
	unicode.cpp
	unicode.h
	util.cpp
	util.h
)

if (UNIX)
	list(APPEND SOURCES xdg.cpp xdg.h)

	if (IRCCD_SYSTEM_LINUX)
		list(APPEND LIBRARIES dl)
	endif ()
elseif (WIN32)
	list(APPEND LIBRARIES ws2_32)
	list(APPEND LIBRARIES gdi32)
	list(APPEND LIBRARIES shlwapi)
endif ()

irccd_define_library(
	TARGET common
	SOURCES ${SOURCES}
	LIBRARIES
		extern-jansson
		${LIBRARIES}
	LOCAL_INCLUDES ${INCLUDES}
	PUBLIC_INCLUDES
		${CMAKE_BINARY_DIR}
		${common_SOURCE_DIR}
		${Jansson_INCLUDE_DIRS}
)

target_compile_definitions(common PUBLIC SOCKET_NO_SSL)

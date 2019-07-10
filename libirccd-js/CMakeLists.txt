#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

project(libirccd-js)

find_package(Boost REQUIRED QUIET COMPONENTS timer)

set(
	SOURCES
	${PROJECT_SOURCE_DIR}/irccd/js.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/chrono_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/chrono_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/directory_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/directory_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/duk.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/duk.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/file_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/file_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/irccd_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/irccd_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/logger_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/logger_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/plugin.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/plugin.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/plugin_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/plugin_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/server_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/server_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/system_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/system_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/timer_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/timer_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/unicode.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/unicode.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/unicode_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/unicode_api.hpp
	${PROJECT_SOURCE_DIR}/irccd/js/util_api.cpp
	${PROJECT_SOURCE_DIR}/irccd/js/util_api.hpp
)

irccd_define_library(
	TARGET libirccd-js
	EXPORT
	HEADERS ${PROJECT_SOURCE_DIR}/irccd/
	SOURCES ${SOURCES}
	LIBRARIES
		Boost::timer
		libduktape
		libirccd-daemon
	PUBLIC_INCLUDES
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>

)

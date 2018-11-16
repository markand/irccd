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

project(libirccd-js)

find_package(Boost REQUIRED QUIET COMPONENTS timer)

set(
	SOURCES
	${libirccd-js_SOURCE_DIR}/irccd/js.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/directory_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/directory_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/duk.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/duk.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/elapsed_timer_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/elapsed_timer_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/file_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/file_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/irccd_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/irccd_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/js_plugin.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/js_plugin.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/logger_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/logger_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/plugin_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/plugin_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/server_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/server_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/system_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/system_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/timer_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/timer_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/unicode.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/unicode.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/unicode_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/unicode_js_api.hpp
	${libirccd-js_SOURCE_DIR}/irccd/js/util_js_api.cpp
	${libirccd-js_SOURCE_DIR}/irccd/js/util_js_api.hpp
)

irccd_define_library(
	TARGET libirccd-js
	EXPORT
	HEADERS ${libirccd-js_SOURCE_DIR}/irccd/
	SOURCES
		${SOURCES}
	LIBRARIES
		Boost::timer
		libduktape
		libirccd-daemon
	PUBLIC_INCLUDES
		$<BUILD_INTERFACE:${libirccd-js_SOURCE_DIR}>

)

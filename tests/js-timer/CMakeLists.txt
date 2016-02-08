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

irccd_define_test(
	NAME js-timer
	SOURCES
		${irccd_SOURCE_DIR}/irccd.cpp
		${irccd_SOURCE_DIR}/irccd.h
		${irccd_SOURCE_DIR}/js.cpp
		${irccd_SOURCE_DIR}/js.h
		${irccd_SOURCE_DIR}/js-directory.cpp
		${irccd_SOURCE_DIR}/js-directory.h
		${irccd_SOURCE_DIR}/js-elapsed-timer.cpp
		${irccd_SOURCE_DIR}/js-elapsed-timer.h
		${irccd_SOURCE_DIR}/js-file.cpp
		${irccd_SOURCE_DIR}/js-file.h
		${irccd_SOURCE_DIR}/js-irccd.cpp
		${irccd_SOURCE_DIR}/js-irccd.h
		${irccd_SOURCE_DIR}/js-logger.cpp
		${irccd_SOURCE_DIR}/js-logger.h
		${irccd_SOURCE_DIR}/js-plugin.cpp
		${irccd_SOURCE_DIR}/js-plugin.h
		${irccd_SOURCE_DIR}/js-server.cpp
		${irccd_SOURCE_DIR}/js-system.cpp
		${irccd_SOURCE_DIR}/js-system.h
		${irccd_SOURCE_DIR}/js-timer.cpp
		${irccd_SOURCE_DIR}/js-timer.h
		${irccd_SOURCE_DIR}/js-unicode.cpp
		${irccd_SOURCE_DIR}/js-util.cpp
		${irccd_SOURCE_DIR}/js-util.h
		${irccd_SOURCE_DIR}/plugin.cpp
		${irccd_SOURCE_DIR}/plugin.h
		${irccd_SOURCE_DIR}/rule.cpp
		${irccd_SOURCE_DIR}/rule.h
		${irccd_SOURCE_DIR}/server.cpp
		${irccd_SOURCE_DIR}/server.h
		${irccd_SOURCE_DIR}/server-state.cpp
		${irccd_SOURCE_DIR}/server-state.h
		${irccd_SOURCE_DIR}/timer.cpp
		${irccd_SOURCE_DIR}/timer.h
		${irccd_SOURCE_DIR}/transport-client.cpp
		${irccd_SOURCE_DIR}/transport-client.h
		${irccd_SOURCE_DIR}/transport-server.cpp
		${irccd_SOURCE_DIR}/transport-server.h
		main.cpp
	RESOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/timer-single.js
		${CMAKE_CURRENT_SOURCE_DIR}/timer-repeat.js
		${CMAKE_CURRENT_SOURCE_DIR}/timer-pending.js
	LIBRARIES common extern-duktape extern-ircclient
)

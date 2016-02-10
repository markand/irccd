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

project(irccd)

set(
	SOURCES
	command-plugin-info.cpp
	command-plugin-info.h
	command-plugin-list.cpp
	command-plugin-list.h
	command-plugin-load.cpp
	command-plugin-load.h
	command-plugin-reload.cpp
	command-plugin-reload.h
	command-plugin-unload.cpp
	command-plugin-unload.h
	command-server-cmode.cpp
	command-server-cmode.h
	command-server-cnotice.cpp
	command-server-cnotice.h
	command-server-connect.cpp
	command-server-connect.h
	command-server-disconnect.cpp
	command-server-disconnect.h
	command-server-info.cpp
	command-server-info.h
	command-server-invite.cpp
	command-server-invite.h
	command-server-join.cpp
	command-server-join.h
	command-server-kick.cpp
	command-server-kick.h
	command-server-list.cpp
	command-server-list.h
	command-server-me.cpp
	command-server-me.h
	command-server-message.cpp
	command-server-message.h
	command-server-mode.cpp
	command-server-mode.h
	command-server-nick.cpp
	command-server-nick.h
	command-server-notice.cpp
	command-server-notice.h
	command-server-part.cpp
	command-server-part.h
	command-server-reconnect.cpp
	command-server-reconnect.h
	command-server-topic.cpp
	command-server-topic.h
	config.cpp
	config.h
	irccd.cpp
	irccd.h
	main.cpp
	rule.cpp
	rule.h
	server.cpp
	server.h
	server-state.cpp
	server-state.h
	transport-command.h
	transport-server.cpp
	transport-server.h
	transport-client.cpp
	transport-client.h
)

if (WITH_JS)
	list(
		APPEND
		SOURCES
		js.cpp
		js.h
		js-directory.cpp
		js-directory.h
		js-elapsed-timer.cpp
		js-elapsed-timer.h
		js-file.cpp
		js-file.h
		js-irccd.cpp
		js-irccd.h
		js-logger.cpp
		js-logger.h
		js-plugin.cpp
		js-plugin.h
		js-server.cpp
		js-server.h
		js-system.cpp
		js-system.h
		js-timer.cpp
		js-timer.h
		js-unicode.cpp
		js-unicode.h
		js-util.cpp
		js-util.h
		plugin.cpp
		plugin.h
		timer.cpp
		timer.h
	)

	list(APPEND LIBRARIES extern-duktape)
endif ()

if (IRCCD_SYSTEM_MAC)
	list(APPEND LIBRARIES resolv)
endif ()

irccd_define_executable(
	TARGET irccd
	INSTALL
	SOURCES ${SOURCES}
	INCLUDES ${irccd_SOURCE_DIR}
	LIBRARIES
		${LIBRARIES}
		extern-ircclient
		common
)

if (UNIX)
	set_target_properties(irccd PROPERTIES LINK_FLAGS "-pthread")
endif ()

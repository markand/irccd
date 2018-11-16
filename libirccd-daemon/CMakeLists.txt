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

project(libirccd-daemon)

set(
	SOURCES
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/bot.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/bot.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/command.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/command.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/dynlib_plugin.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/dynlib_plugin.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/irc.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/irc.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/logger.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/logger.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/plugin.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/plugin.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/plugin_service.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/plugin_service.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/rule.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/rule.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/rule_service.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/rule_service.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/rule_util.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/rule_util.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/server.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/server.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/server_service.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/server_service.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/server_util.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/server_util.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/transport_client.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/transport_client.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/transport_server.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/transport_server.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/transport_service.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/transport_service.hpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/transport_util.cpp
	${libirccd-daemon_SOURCE_DIR}/irccd/daemon/transport_util.hpp
)

irccd_define_library(
	TARGET libirccd-daemon
	EXPORT
	HEADERS ${libirccd-daemon_SOURCE_DIR}/irccd/
	SOURCES ${SOURCES}
	LIBRARIES libirccd
	PUBLIC_INCLUDES $<BUILD_INTERFACE:${libirccd-daemon_SOURCE_DIR}>
)

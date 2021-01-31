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

project(libirccd)

set(
	SOURCES
	CMakeLists.txt
	irccd/channel.c
	irccd/channel.h
	irccd/config.h.in
	irccd/conn.c
	irccd/conn.h
	irccd/event.c
	irccd/event.h
	irccd/irccd.c
	irccd/irccd.h
	irccd/limits.h
	irccd/log.c
	irccd/log.h
	irccd/plugin.c
	irccd/plugin.h
	irccd/rule.c
	irccd/rule.h
	irccd/server.c
	irccd/server.h
	irccd/subst.c
	irccd/subst.h
	irccd/util.c
	irccd/util.h
)

configure_file(
	${libirccd_SOURCE_DIR}/irccd/config.h.in
	${libirccd_BINARY_DIR}/irccd/config.h
)

add_library(libirccd ${SOURCES})
set_target_properties(libirccd PROPERTIES PREFIX "")

target_include_directories(
	libirccd
	PUBLIC
		$<BUILD_INTERFACE:${libirccd_SOURCE_DIR}>
		$<BUILD_INTERFACE:${libirccd_BINARY_DIR}>/irccd
)
target_link_libraries(libirccd libirccd-compat)

if (IRCCD_WITH_SSL)
	target_link_libraries(libirccd OpenSSL::SSL OpenSSL::Crypto)
endif ()

source_group(TREE ${libirccd_SOURCE_DIR} FILES ${SOURCES})

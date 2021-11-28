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

project(libirccd)

set(
	LIBBSD_SOURCES
	${irccd_SOURCE_DIR}/extern/libbsd/strlcpy.c
	${irccd_SOURCE_DIR}/extern/libbsd/strlcat.c
	${irccd_SOURCE_DIR}/extern/libbsd/reallocarray.c
)

set(
	HEADERS
	${libirccd_SOURCE_DIR}/irccd/channel.h
	${libirccd_SOURCE_DIR}/irccd/event.h
	${libirccd_SOURCE_DIR}/irccd/hook.h
	${libirccd_SOURCE_DIR}/irccd/irccd.h
	${libirccd_SOURCE_DIR}/irccd/limits.h
	${libirccd_SOURCE_DIR}/irccd/log.h
	${libirccd_SOURCE_DIR}/irccd/plugin.h
	${libirccd_SOURCE_DIR}/irccd/rule.h
	${libirccd_SOURCE_DIR}/irccd/server.h
	${libirccd_SOURCE_DIR}/irccd/subst.h
	${libirccd_SOURCE_DIR}/irccd/util.h
)

set(
	PRIVHEADERS
	${libirccd_SOURCE_DIR}/irccd/conn.h
)

set(
	SOURCES
	${libirccd_SOURCE_DIR}/irccd/channel.c
	${libirccd_SOURCE_DIR}/irccd/conn.c
	${libirccd_SOURCE_DIR}/irccd/event.c
	${libirccd_SOURCE_DIR}/irccd/hook.c
	${libirccd_SOURCE_DIR}/irccd/irccd.c
	${libirccd_SOURCE_DIR}/irccd/log.c
	${libirccd_SOURCE_DIR}/irccd/plugin.c
	${libirccd_SOURCE_DIR}/irccd/rule.c
	${libirccd_SOURCE_DIR}/irccd/server.c
	${libirccd_SOURCE_DIR}/irccd/subst.c
	${libirccd_SOURCE_DIR}/irccd/util.c
)

configure_file(
	${libirccd_SOURCE_DIR}/irccd/config.h.in
	${libirccd_BINARY_DIR}/irccd/config.h
)

add_library(libirccd STATIC ${LIBBSD_SOURCES} ${SOURCES} ${HEADERS} ${PRIVHEADERS})
target_link_libraries(libirccd PUBLIC libirccd-utlist)
target_include_directories(
	libirccd
	PUBLIC
		$<BUILD_INTERFACE:${libirccd_SOURCE_DIR}>
		$<BUILD_INTERFACE:${libirccd_BINARY_DIR}>
		$<BUILD_INTERFACE:${libirccd_BINARY_DIR}/irccd>
	PRIVATE
		$<BUILD_INTERFACE:${libirccd_BINARY_DIR}/irccd>
)
set_target_properties(libirccd PROPERTIES PREFIX "")

if (IRCCD_WITH_JS)
	target_link_libraries(libirccd PUBLIC libirccd-duktape)
endif ()
if (IRCCD_WITH_SSL)
	target_link_libraries(libirccd PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif ()
if (IRCCD_WITH_PKGCONFIG)
	configure_file(
		${libirccd_SOURCE_DIR}/irccd.pc.in
		${libirccd_BINARY_DIR}/irccd.pc
	)
	install(
		FILES ${libirccd_BINARY_DIR}/irccd.pc
		DESTINATION ${IRCCD_WITH_PKGCONFIGDIR}
	)
endif ()

install(
	FILES ${HEADERS}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/irccd
)
install(
	FILES ${libirccd_BINARY_DIR}/irccd/config.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/irccd
)

source_group(extern/libbsd FILES ${LIBBSD_SOURCES})
source_group(irccd FILES ${SOURCES} ${HEADERS})

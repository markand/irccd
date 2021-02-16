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

project(irccd)

set(
	SOURCES
	${irccd_SOURCE_DIR}/dl-plugin.c
	${irccd_SOURCE_DIR}/dl-plugin.h
	${irccd_SOURCE_DIR}/peer.c
	${irccd_SOURCE_DIR}/peer.h
	${irccd_SOURCE_DIR}/transport.c
	${irccd_SOURCE_DIR}/transport.h
)

if (IRCCD_WITH_JS)
	list(
		APPEND SOURCES
		${irccd_SOURCE_DIR}/js-plugin.c
		${irccd_SOURCE_DIR}/js-plugin.h
		${irccd_SOURCE_DIR}/jsapi-chrono.c
		${irccd_SOURCE_DIR}/jsapi-chrono.h
		${irccd_SOURCE_DIR}/jsapi-directory.c
		${irccd_SOURCE_DIR}/jsapi-directory.h
		${irccd_SOURCE_DIR}/jsapi-file.c
		${irccd_SOURCE_DIR}/jsapi-file.h
		${irccd_SOURCE_DIR}/jsapi-hook.c
		${irccd_SOURCE_DIR}/jsapi-hook.h
		${irccd_SOURCE_DIR}/jsapi-irccd.c
		${irccd_SOURCE_DIR}/jsapi-irccd.h
		${irccd_SOURCE_DIR}/jsapi-logger.c
		${irccd_SOURCE_DIR}/jsapi-logger.h
		${irccd_SOURCE_DIR}/jsapi-plugin.c
		${irccd_SOURCE_DIR}/jsapi-plugin.h
		${irccd_SOURCE_DIR}/jsapi-rule.c
		${irccd_SOURCE_DIR}/jsapi-rule.h
		${irccd_SOURCE_DIR}/jsapi-server.c
		${irccd_SOURCE_DIR}/jsapi-server.h
		${irccd_SOURCE_DIR}/jsapi-system.c
		${irccd_SOURCE_DIR}/jsapi-system.h
		${irccd_SOURCE_DIR}/jsapi-timer.c
		${irccd_SOURCE_DIR}/jsapi-timer.h
		${irccd_SOURCE_DIR}/jsapi-unicode.c
		${irccd_SOURCE_DIR}/jsapi-unicode.h
		${irccd_SOURCE_DIR}/jsapi-util.c
		${irccd_SOURCE_DIR}/jsapi-util.h
		${irccd_SOURCE_DIR}/unicode.c
		${irccd_SOURCE_DIR}/unicode.h
	)
endif ()

# This is required for unit tests.
add_library(irccd-fe OBJECT ${SOURCES})
target_link_libraries(irccd-fe libirccd-static)
target_include_directories(
	irccd-fe
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
		$<BUILD_INTERFACE:${irccd_SOURCE_DIR}>
)

if (IRCCD_WITH_JS)
	target_link_libraries(irccd-fe libirccd-duktape -pthread ${CMAKE_DL_LIBS})
endif ()

flex_target(irccd-lex ${irccd_SOURCE_DIR}/lex.l ${irccd_BINARY_DIR}/lex.c)
bison_target(irccd-conf ${irccd_SOURCE_DIR}/conf.y ${irccd_BINARY_DIR}/conf.c)
add_flex_bison_dependency(irccd-lex irccd-conf)

add_executable(
	irccd
	${irccd_SOURCE_DIR}/main.c
	${irccd_SOURCE_DIR}/lex.l
	${irccd_SOURCE_DIR}/conf.y
	${BISON_irccd-conf_OUTPUTS}
	${FLEX_irccd-lex_OUTPUTS}
)

set_target_properties(irccd PROPERTIES ENABLE_EXPORTS On)
target_link_libraries(irccd irccd-fe)
install(TARGETS irccd DESTINATION ${CMAKE_INSTALL_BINDIR})

source_group(TREE ${irccd_SOURCE_DIR} FILES ${SOURCES})

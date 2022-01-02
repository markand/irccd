#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
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
	${irccd_SOURCE_DIR}/unicode.c
	${irccd_SOURCE_DIR}/unicode.h
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
	)
endif ()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads REQUIRED)

bison_target(yacc ${irccd_SOURCE_DIR}/conf.y
	${irccd_BINARY_DIR}/conf.c
	DEFINES_FILE ${irccd_BINARY_DIR}/conf.h
)
flex_target(lex ${irccd_SOURCE_DIR}/lex.l ${irccd_BINARY_DIR}/lex.c)
add_flex_bison_dependency(lex yacc)

#
# libirccd-static
# -------------------------------------------------------------------
#
add_library(
	irccd-static OBJECT
	${SOURCES}
	${irccd_BINARY_DIR}/lex.c
	${irccd_BINARY_DIR}/conf.c
	${irccd_BINARY_DIR}/conf.h
)
target_include_directories(irccd-static
	PRIVATE $<BUILD_INTERFACE:${irccd_SOURCE_DIR}>
	PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
)
target_link_libraries(
	irccd-static
	${CMAKE_DL_LIBS}
	Threads::Threads
	libirccd
)
set_target_properties(irccd-static PROPERTIES PREFIX "")

#
# irccd(1)
# -------------------------------------------------------------------
#
add_executable(irccd ${irccd_SOURCE_DIR}/main.c)
target_link_libraries(irccd irccd-static)
set_target_properties(irccd PROPERTIES ENABLE_EXPORTS On)

# Installation.
install(
	TARGETS irccd
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Source grouping.
source_group(TREE ${irccd_SOURCE_DIR} FILES ${SOURCES} ${irccd_SOURCE_DIR}/main.c)
source_group(
	build
	FILES
		${irccd_BINARY_DIR}/lex.c
		${irccd_BINARY_DIR}/conf.c
		${irccd_BINARY_DIR}/conf.h
)

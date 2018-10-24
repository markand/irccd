#
# CMakeLists.txt -- CMake build system for uriparser
#
# Copyright (c) 2016-2018 David Demelier <markand@malikania.fr>
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

cmake_minimum_required(VERSION 3.5)
project(liburiparser C)

set(
	HEADERS
	${liburiparser_SOURCE_DIR}/include/uriparser/UriBase.h
	${liburiparser_SOURCE_DIR}/include/uriparser/UriDefsAnsi.h
	${liburiparser_SOURCE_DIR}/include/uriparser/UriDefsConfig.h
	${liburiparser_SOURCE_DIR}/include/uriparser/UriDefsUnicode.h
	${liburiparser_SOURCE_DIR}/include/uriparser/Uri.h
	${liburiparser_SOURCE_DIR}/include/uriparser/UriIp4.h
)

set(
	SOURCES
	${liburiparser_SOURCE_DIR}/src/UriCommon.c
	${liburiparser_SOURCE_DIR}/src/UriCommon.h
	${liburiparser_SOURCE_DIR}/src/UriCompare.c
	${liburiparser_SOURCE_DIR}/src/UriEscape.c
	${liburiparser_SOURCE_DIR}/src/UriFile.c
	${liburiparser_SOURCE_DIR}/src/UriIp4Base.c
	${liburiparser_SOURCE_DIR}/src/UriIp4Base.h
	${liburiparser_SOURCE_DIR}/src/UriIp4.c
	${liburiparser_SOURCE_DIR}/src/UriNormalizeBase.c
	${liburiparser_SOURCE_DIR}/src/UriNormalizeBase.h
	${liburiparser_SOURCE_DIR}/src/UriNormalize.c
	${liburiparser_SOURCE_DIR}/src/UriParseBase.c
	${liburiparser_SOURCE_DIR}/src/UriParseBase.h
	${liburiparser_SOURCE_DIR}/src/UriParse.c
	${liburiparser_SOURCE_DIR}/src/UriQuery.c
	${liburiparser_SOURCE_DIR}/src/UriRecompose.c
	${liburiparser_SOURCE_DIR}/src/UriResolve.c
	${liburiparser_SOURCE_DIR}/src/UriShorten.c
)

add_library(liburiparser ${HEADERS} ${SOURCES})

target_include_directories(
	liburiparser
	PUBLIC
		$<BUILD_INTERFACE:${liburiparser_SOURCE_DIR}/include>
)
set_target_properties(liburiparser PROPERTIES PREFIX "")

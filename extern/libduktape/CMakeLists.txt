#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2024 David Demelier <markand@malikania.fr>
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

project(libirccd-duktape)

set(
	SOURCES
	${libirccd-duktape_SOURCE_DIR}/duktape.c
	${libirccd-duktape_SOURCE_DIR}/duk_config.h
	${libirccd-duktape_SOURCE_DIR}/duktape.h
)

add_library(libirccd-duktape STATIC ${SOURCES})
target_include_directories(
	libirccd-duktape
	PUBLIC
		$<BUILD_INTERFACE:${libirccd-duktape_SOURCE_DIR}>
)
set_target_properties(libirccd-duktape PROPERTIES PREFIX "" FOLDER extern)

if (M_LIBRARY)
	target_link_libraries(libirccd-duktape ${M_LIBRARY})
endif ()

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

project(lib)

include(irccd/CMakeSources.cmake)

add_library(libirccd ${HEADERS} ${SOURCES})

target_include_directories(
	libirccd
	PUBLIC
		${irccd_BINARY_DIR}
		${OPENSSL_INCLUDE_DIR}
		${lib_SOURCE_DIR}
)

if (IRCCD_SYSTEM_WINDOWS)
	list(APPEND LIBRARIES ws2_32 shlwapi)
elseif (IRCCD_SYSTEM_MAC)
	list(APPEND LIBRARIES resolv)
endif ()

target_link_libraries(libirccd extern-duktape extern-ircclient extern-jansson ${LIBRARIES})

set_target_properties(
	libirccd
	PROPERTIES
		PREFIX ""
		OUTPUT_NAME_DEBUG libirccd2d
		VERSION ${IRCCD_VERSION}
		SOVERSION ${IRCCD_VERSION_SHLIB}
)

install(
	TARGETS libirccd
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# pkg-config
if (WITH_PKGCONFIG)
	# Configure the pkg-config file.
	configure_file(
		${CMAKE_SOURCE_DIR}/cmake/internal/irccd2.pc.in
		${CMAKE_BINARY_DIR}/irccd2.pc.tmp
		@ONLY
	)

	# Adjust the library output name if debug.
	file(
		GENERATE
		OUTPUT ${CMAKE_BINARY_DIR}/irccd2.pc
		INPUT ${CMAKE_BINARY_DIR}/irccd2.pc.tmp
	)

	install(FILES ${CMAKE_BINARY_DIR}/irccd2.pc DESTINATION ${WITH_PKGCONFIGDIR})
endif ()

#
# IrccdDefineLibrary.cmake -- CMake build system for irccd
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

#
# irccd_define_library
# --------------------
#
# irccd_define_library(
#   TARGET              target name
#   SOURCES             src1, src2, srcn
#   EXPORT              (Optional) set to true to export library through irccd
#   HEADERS             (Optional) headers to install
#   HEADERS_DIRECTORY   (Optional) subdirectory where to install headers
#   FLAGS               (Optional) C/C++ flags (without -D)
#   LIBRARIES           (Optional) libraries to link
#   LOCAL_INCLUDES      (Optional) local includes for the target only
#   PUBLIC_INCLUDES     (Optional) includes to share with target dependencies
# )
#

include(${CMAKE_CURRENT_LIST_DIR}/IrccdVeraCheck.cmake)

function(irccd_define_library)
	set(options EXPORT)
	set(oneValueArgs HEADERS_DIRECTORY TARGET)
	set(multiValueArgs HEADERS SOURCES FLAGS LIBRARIES LOCAL_INCLUDES PUBLIC_INCLUDES)

	cmake_parse_arguments(LIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT LIB_TARGET)
		message(FATAL_ERROR "Please set TARGET")
	endif ()
	if (NOT LIB_SOURCES)
		message(FATAL_ERROR "Please set SOURCES")
	endif ()

	add_library(${LIB_TARGET} ${LIB_SOURCES} ${LIB_HEADERS})
	target_include_directories(${LIB_TARGET} PRIVATE ${LIB_LOCAL_INCLUDES} PUBLIC ${LIB_PUBLIC_INCLUDES})
	target_compile_definitions(
		${LIB_TARGET}
		PRIVATE
			CMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
			CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
		PUBLIC
			${LIB_FLAGS}
	)
	target_link_libraries(${LIB_TARGET} ${LIB_LIBRARIES})
	set_target_properties(
		${LIB_TARGET}
		PROPERTIES
			PREFIX ""
			RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	)
	foreach (c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${c} cu)
		set_target_properties(
			${LIB_TARGET}
			PROPERTIES
				RUNTIME_OUTPUT_DIRECTORY_${cu} ${CMAKE_BINARY_DIR}/bin/${c}
		)
	endforeach()

	if (${LIB_EXPORT})
		install(
			TARGETS ${LIB_TARGET}
			EXPORT irccd-targets
			RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
			ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
			LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		)
	endif ()

	if (LIB_HEADERS)
		if (NOT LIB_HEADERS_DIRECTORY)
			message(FATAL_ERROR "HEADERS_DIRECTORY must be defined")
		endif ()

		install(
			FILES ${LIB_HEADERS}
			DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_HEADERS_DIRECTORY}
		)
	endif ()
endfunction()

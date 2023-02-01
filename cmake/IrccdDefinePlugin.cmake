#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2023 David Demelier <markand@malikania.fr>
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

include(GNUInstallDirs)

function(irccd_define_native_plugin)
	set(options "")
	set(oneValueArgs "MAN;NAME")
	set(multiValueArgs "FLAGS;INCLUDES;LIBRARIES;SOURCES")

	cmake_parse_arguments(PLG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT PLG_NAME)
		message(FATAL_ERROR "Missing NAME")
	elseif (NOT PLG_SOURCES)
		message(FATAL_ERROR "Missing SOURCES")
	endif ()

	add_library(irccd-plugin-${PLG_NAME} MODULE ${PLG_SOURCES} ${PLG_MAN})
	target_link_libraries(irccd-plugin-${PLG_NAME} irccd)

	if (PLG_FLAGS)
		target_compile_definitions(irccd-plugin-${PLG_NAME} ${PLG_FLAGS})
	endif ()
	if (PLG_INCLUDES)
		target_include_directories(irccd-plugin-${PLG_NAME} ${PLG_INCLUDES})
	endif ()
	if (PLG_LIBRARIES)
		target_link_libraries(irccd-plugin-${PLG_NAME} ${PLG_LIBRARIES})
	endif ()

	if (APPLE)
		target_link_libraries(irccd-plugin-${PLG_NAME} "-undefined dynamic_lookup")
	endif ()

	set_target_properties(
		irccd-plugin-${PLG_NAME}
		PROPERTIES
			OUTPUT_NAME ${PLG_NAME}
			PREFIX ""
			FOLDER plugins
	)

	install(
		TARGETS irccd-plugin-${PLG_NAME}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/irccd
	)

	if (PLG_MAN)
		install(
			FILES ${PLG_MAN}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
			RENAME irccd-plugin-${PLG_NAME}.7
		)
	endif ()
endfunction()

function(irccd_define_javascript_plugin)
	set(options "")
	set(oneValueArgs "MAN;NAME")
	set(multiValueArgs "SCRIPT")

	cmake_parse_arguments(PLG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	add_custom_target(irccd-plugin-${PLG_NAME} SOURCES ${PLG_SCRIPT})
	set_target_properties(irccd-plugin-${PLG_NAME} PROPERTIES FOLDER plugins)

	cmake_path(GET PLG_SCRIPT FILENAME filename)
	configure_file(
		${PLG_SCRIPT}
		${CMAKE_CURRENT_BINARY_DIR}/${filename}
		@ONLY
	)
	install(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/${filename}
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/irccd
	)

	if (PLG_MAN)
		install(
			FILES ${PLG_MAN}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
			RENAME irccd-plugin-${PLG_NAME}.7
		)
	endif ()
endfunction()

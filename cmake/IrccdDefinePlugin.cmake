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

function(irccd_define_js_plugin)
	set(options "")
	set(oneValueArgs MAN NAME SCRIPT)
	set(multiValueArgs "")

	cmake_parse_arguments(PLG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT PLG_NAME)
		message(FATAL_ERROR "Missing NAME argument")
	elseif (NOT PLG_SCRIPT)
		message(FATAL_ERROR "Missing SCRIPT argument")
	endif ()

	# Install script.
	get_filename_component(basename ${PLG_SCRIPT} NAME)
	configure_file(${PLG_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${basename})

	install(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/${basename}
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/irccd
	)

	# Install manual page.
	if (PLG_MAN)
		get_filename_component(basename ${PLG_MAN} NAME)
		configure_file(${PLG_MAN} ${CMAKE_CURRENT_BINARY_DIR}/${basename})

		install(
			FILES ${CMAKE_CURRENT_BINARY_DIR}/${basename}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
			RENAME irccd-plugin-${basename}
		)
	endif ()
endfunction()

function(irccd_define_c_plugin)
	set(options "")
	set(oneValueArgs NAME MAN)
	set(multiValueArgs LIBRARIES SOURCES)

	cmake_parse_arguments(PLG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT PLG_NAME)
		message(FATAL_ERROR "Missing NAME argument")
	elseif (NOT PLG_SOURCES)
		message(FATAL_ERROR "Missing SOURCES argument")
	endif ()

	add_library(${PLG_NAME} MODULE ${PLG_SOURCES})
	get_target_property(LIBIRCCD_INCLUDES libirccd INCLUDE_DIRECTORIES)
	get_target_property(LIBCOMPAT_INCLUDES libirccd-compat INCLUDE_DIRECTORIES)
	target_include_directories(
		${PLG_NAME}
		PRIVATE
			${LIBIRCCD_INCLUDES}
			${LIBCOMPAT_INCLUDES}
			${OPENSSL_INCLUDE_DIR}
	)
	target_link_libraries(${PLG_NAME} ${PLG_LIBRARIES})
	set_target_properties(${PLG_NAME} PROPERTIES PREFIX "")

	#
	# This is required but not enabled by default, otherwise we get
	# undefined errors from any libirccd functions.
	#
	if (APPLE)
		set_target_properties(${PLG_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
	endif ()

	if (PLG_MAN)
		get_filename_component(basename ${PLG_MAN} NAME)
		configure_file(${PLG_MAN} ${CMAKE_CURRENT_BINARY_DIR}/${basename})

		install(
			FILES ${CMAKE_CURRENT_BINARY_DIR}/${basename}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
			RENAME irccd-plugin-${basename}
		)
	endif ()
endfunction()

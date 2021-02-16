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

include(GNUInstallDirs)

function(_idp_install_man file)
	get_filename_component(basename ${file} NAME)
	configure_file(${file} ${CMAKE_CURRENT_BINARY_DIR}/${basename})

	install(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/${basename}
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
		RENAME irccd-plugin-${basename}
	)
endfunction ()

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

	# Create a dummy custom target just to get it through the IDE.
	add_custom_target(
		irccd-plugin-${PLG_NAME}
		SOURCES ${PLG_SCRIPT} ${PLG_MAN}
	)
	set_target_properties(irccd-plugin-${PLG_NAME} PROPERTIES FOLDER "plugins")

	# Install script.
	get_filename_component(basename ${PLG_SCRIPT} NAME)
	configure_file(${PLG_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${basename})

	install(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/${basename}
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/irccd
	)

	if (PLG_MAN)
		_idp_install_man(${PLG_MAN})
	endif ()
endfunction()

function(irccd_define_c_plugin)
	set(options "")
	set(oneValueArgs NAME MAN)
	set(multiValueArgs INCLUDES LIBRARIES SOURCES)

	cmake_parse_arguments(PLG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT PLG_NAME)
		message(FATAL_ERROR "Missing NAME argument")
	elseif (NOT PLG_SOURCES)
		message(FATAL_ERROR "Missing SOURCES argument")
	endif ()

	add_library(irccd-plugin-${PLG_NAME} MODULE ${PLG_SOURCES} ${PLG_MAN})
	target_link_libraries(irccd-plugin-${PLG_NAME} irccd::libirccd ${PLG_LIBRARIES})
	set_target_properties(irccd-plugin-${PLG_NAME}
		PROPERTIES
			PREFIX ""
			PROJECT_LABEL "irccd"
			FOLDER "plugins"
			OUTPUT_NAME ${PLG_NAME}
			RUNTIME_OUTPUT_NAME_${c} ${PLG_NAME}
	)
	install(TARGETS irccd-plugin-${PLG_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/irccd)

	foreach (c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${c} c)
		set_target_properties(irccd-plugin-${PLG_NAME}
			PROPERTIES
				OUTPUT_NAME_${c} ${PLG_NAME}
				RUNTIME_OUTPUT_NAME_${c} ${PLG_NAME}
		)
	endforeach ()

	#
	# This is required but not enabled by default, otherwise we get
	# undefined errors from any libirccd functions.
	#
	if (APPLE)
		set_target_properties(irccd-plugin-${PLG_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
	endif ()

	if (PLG_MAN)
		_idp_install_man(${PLG_MAN})
	endif ()
endfunction()

#
# IrccdDefinePlugin.cmake -- CMake build system for irccd
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
# irccd_define_plugin
# -------------------
#
# Synopsis for Javascript plugins.
#
# irccd_define_plugin(
#   NAME        canonical plugin name
#   TYPE        JS
#   DOCS        documentation files in markdown
#   SCRIPT      absolute path to the Javascript file (ending with .js)
# )
#
# Synopsis for native plugins.
#
# irccd_define_plugin(
#   NAME        canonical plugin name
#   TYPE        NATIVE
#   DOCS        documentation files in markdown
#   SOURCES     c++ source files
#   INCLUDES    additional includes
#   LIBRARIES   additional libraries
# )
#
# Create a Javascript or native plugin.
#
# The NAME parameter identifies the plugin. The same name will be used for the
# plugin filename.
#
# Both Javascript and native plugins are supported specified by the TYPE
# parameter to JS or NATIVE respectively. For Javascript plugin, a unique file
# must be given as SCRIPT parameter. For native plugins, any source files can
# be given as SOURCES parameter.
#
# Additional documentation can be built in markdown and installed along the
# plugin using DOCS parameter.
#
# A CMake option is also created in the form OPTION_<PLG> where PLG is the
# uppercase NAME value.
#

include(${CMAKE_CURRENT_LIST_DIR}/IrccdInstallDependencies.cmake)

function(_irccd_define_javascript_plugin)
	if (NOT PLG_SCRIPT)
		message(FATAL_ERROR "Missing SCRIPT parameter")
	endif ()

	get_filename_component(name ${PLG_SCRIPT} NAME)

	configure_file(
		${PLG_SCRIPT}
		${CMAKE_CURRENT_BINARY_DIR}/${name}
	)

	install(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}
		COMPONENT ${PLG_NAME}
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/irccd
	)

	add_custom_target(
		plugin-${PLG_NAME}
		ALL
		DEPENDS ${PLG_OUTPUT_DOC}
		SOURCES ${PLG_SCRIPT} ${PLG_DOCS}
	)
endfunction()

function(_irccd_define_native_plugin)
	if (NOT PLG_SOURCES)
		message(FATAL_ERROR "Missing SOURCES parameter")
	endif ()

	add_library(plugin-${PLG_NAME} MODULE ${PLG_SOURCES} ${PLG_OUTPUT_DOC} ${PLG_DOCS})
	target_link_libraries(plugin-${PLG_NAME} libirccd-daemon ${PLG_LIBRARIES})
	target_include_directories(plugin-${PLG_NAME} PRIVATE ${PLG_INCLUDES})

	# Change output name.
	set_target_properties(
		plugin-${PLG_NAME}
		PROPERTIES
			PREFIX ""
			OUTPUT_NAME ${PLG_NAME}
	)
	foreach (cfg ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${cfg} CFG)
		set_target_properties(
			plugin-${PLG_NAME}
			PROPERTIES
				PREFIX ""
				OUTPUT_NAME_${CFG} ${PLG_NAME}
		)
	endforeach ()

	install(
		TARGETS plugin-${PLG_NAME}
		COMPONENT ${PLG_NAME}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/irccd
	)

	irccd_install_dependencies(plugin-${PLG_NAME})
endfunction()

function(irccd_define_plugin)
	set(options "")
	set(oneValueArgs NAME DOCS TYPE SCRIPT)
	set(multiValueArgs SOURCES INCLUDES LIBRARIES)

	cmake_parse_arguments(PLG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT PLG_NAME)
		message(FATAL_ERROR "Missing NAME parameter")
	endif ()

	string(TOUPPER ${PLG_NAME} PLG_UPPER_NAME)
	option(IRCCD_WITH_PLUGIN_${PLG_UPPER_NAME} "Enable ${PLG_NAME} plugin" On)

	if (NOT IRCCD_WITH_PLUGIN_${PLG_UPPER_NAME})
		setg(IRCCD_WITH_PLUGIN_${PLG_UPPER_NAME}_MSG "No (disabled by user)")
	else ()
		setg(IRCCD_WITH_PLUGIN_${PLG_UPPER_NAME}_MSG "Yes")

		# Optional documentation.
		if (PLG_DOCS)
			irccd_build_html(
				SOURCE ${PLG_DOCS}
				COMPONENT ${PLG_NAME}
				OUTPUT_DIR plugin/${PLG_NAME}
				OUTPUT_VAR PLG_OUTPUT_DOC
				TEMPLATE ${html_SOURCE_DIR}/template.html
			)
		endif ()

		if (PLG_TYPE MATCHES "JS")
			_irccd_define_javascript_plugin()
		elseif (PLG_TYPE MATCHES "NATIVE")
			_irccd_define_native_plugin()
		else ()
			message(FATAL_ERROR "Invalid TYPE given, must be JS or NATIVE")
		endif ()

		# Put under "plugins".
		set_target_properties(
			plugin-${PLG_NAME}
			PROPERTIES
				FOLDER "plugins"
				PROJECT_NAME ${PLG_NAME}
		)

		# Component grouping in installer.
		setg(CPACK_COMPONENT_${PLG_UPPER_NAME}_GROUP "Plugins")
		setg(CPACK_COMPONENT_${PLG_UPPER_NAME}_DESCRIPTION "Install ${PLG_NAME} plugin.")
	endif ()
endfunction()

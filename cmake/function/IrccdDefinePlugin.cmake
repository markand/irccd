#
# IrccdDefinePlugin.cmake -- CMake build system for irccd
#
# Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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
# -------------------------------------------------------------------
#
# irccd_define_plugin(
#   NAME canonical plugin name
#   TYPE JS or NATIVE
#   DOCS documentation files in markdown
#
#   Options for TYPE JS:
#
#   SCRIPT absolute path to the Javascript file (ending with .js)
#
#   Options for TYPE NATIVE:
#
#   SOURCES c++ source files
#
# Create a Javascript or Native plugin.
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

function(_irccd_define_javascript_plugin)
    if (NOT PLG_SCRIPT)
        message(FATAL_ERROR "Missing SCRIPT parameter")
    endif ()

    get_filename_component(name ${PLG_SCRIPT} NAME)

    configure_file(
        ${PLG_SCRIPT}
        ${IRCCD_FAKEROOTDIR}/${WITH_PLUGINDIR}/${name}
    )

    install(
        FILES ${IRCCD_FAKEROOTDIR}/${WITH_PLUGINDIR}/${name}
        COMPONENT ${PLG_NAME}
        DESTINATION ${WITH_PLUGINDIR}
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

    # Move the target into the native plugin directory and rename it.
    set_target_properties(
        plugin-${PLG_NAME}
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME ${PLG_NAME}
            LIBRARY_OUTPUT_DIRECTORY ${IRCCD_FAKEROOTDIR}/${WITH_NPLUGINDIR}
    )
    foreach (c ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER CONFIG ${c})
        set_target_properties(
            plugin-${PLG_NAME}
            PROPERTIES
                OUTPUT_NAME_${CONFIG} ${PLG_NAME}
                LIBRARY_OUTPUT_DIRECTORY_${CONFIG} ${IRCCD_FAKEROOTDIR}/${WITH_NPLUGINDIR}
        )
    endforeach()
    target_link_libraries(plugin-${PLG_NAME} libirccd)
    install(
        TARGETS plugin-${PLG_NAME}
        COMPONENT ${PLG_NAME}
        LIBRARY DESTINATION ${WITH_NPLUGINDIR}
    )
endfunction()

function(irccd_define_plugin)
    set(options "")
    set(oneValueArgs NAME DOCS TYPE SCRIPT)
    set(multiValueArgs SOURCES)

    cmake_parse_arguments(PLG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT PLG_NAME)
        message(FATAL_ERROR "Missing NAME parameter")
    endif ()

    string(TOUPPER ${PLG_NAME} PLG_UPPER_NAME)
    option(WITH_PLUGIN_${PLG_UPPER_NAME} "Enable ${PLG_NAME} plugin" On)

    if (NOT WITH_PLUGIN_${PLG_UPPER_NAME})
        setg(WITH_PLUGIN_${PLG_UPPER_NAME}_MSG "No (disabled by user)")
    else ()
        setg(WITH_PLUGIN_${PLG_UPPER_NAME}_MSG "Yes")

        # Optional documentation.
        if (PLG_DOCS AND WITH_HTML)
            irccd_build_html(
                SOURCE ${PLG_DOCS}
                OUTPUT plugin/${PLG_NAME}
                COMPONENT ${PLG_NAME}
                OUTPUT_VAR PLG_OUTPUT_DOC
            )
        endif ()

        if (PLG_TYPE MATCHES "JS")
            _irccd_define_javascript_plugin()
        elseif (PLG_TYPE MATCHES "NATIVE")
            _irccd_define_native_plugin()
        else ()
            message(FATAL_ERROR "Invalid TYPE given, must be JS or NATIVE")
        endif ()

        # Component grouping in installer.
        setg(CPACK_COMPONENT_${PLG_UPPER_NAME}_GROUP "Plugins")
        setg(CPACK_COMPONENT_${PLG_UPPER_NAME}_DESCRIPTION "Install ${PLG_NAME} plugin.")
    endif ()
endfunction()

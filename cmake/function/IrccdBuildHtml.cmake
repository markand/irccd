#
# IrccdBuildHtml.cmake -- CMake build system for irccd
#
# Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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
# irccd_build_html
# ----------------
#
# irccd_build_html(
#   SOURCE              the source markdown file
#   TEMPLATE            specify template file
#   OUTPUT_DIR          the output directory (relative to base doc dir)
#   OUTPUT_VAR          (Optional) store the output file in the output variable
#   COMPONENT           (Optional) install the output as the given component
# )
#
# Create a rule to build the markdown file specified by SOURCE parameter.
#
# The SOURCE file will be written in
# ${CMAKE_BINARY_DIR}/html/${OUTPUT}/<basename>.html where basename is the file
# name taken from SOURCE with extension replaced with html.
#
# This macro does not create a target, just the output rule and the output file
# can be retrieved using the OUTPUT_VAR variable.
#
# If component is set, a CPack component is created for that output file and
# will be grouped together.
#
# Example:
#
# irccd_build_html(
#   SOURCE onMessage.md
#   TEMPLATE template.html
#   COMPONENT documentation
#   OUTPUT_DIR event
#   OUTPUT_VAR myvar
# )
#
# add_custom_target(mytarget DEPENDS ${myvar})
#
# It's perfectly safe to call this macro multiple times with the same COMPONENT.
#

option(IRCCD_WITH_HTML "Enable building of HTML documentation" On)

find_package(marker QUIET)

if (IRCCD_WITH_HTML)
	if (marker_FOUND)
		set(IRCCD_HAVE_HTML On)
		set(IRCCD_WITH_HTML_MSG "Yes")
	else ()
		set(IRCCD_WITH_HTML_MSG "No (marker not found)")
	endif ()
else ()
	set(IRCCD_WITH_HTML_MSG "No (disabled by user)")
endif ()

macro(irccd_build_html)
	if (IRCCD_HAVE_HTML)
		set(options "")
		set(oneValueArgs "COMPONENT;OUTPUT_DIR;OUTPUT_VAR;SOURCE;TEMPLATE")
		set(multiValueArgs "VARIABLES")

		cmake_parse_arguments(HTML "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

		if (NOT HTML_SOURCE)
			message(FATAL_ERROR "Missing SOURCE parameter")
		endif ()
		if (NOT HTML_TEMPLATE)
			message(FATAL_ERROR "Missing TEMPLATE parameter")
		endif ()
		if (IS_ABSOLUTE ${HTML_OUTPUT_DIR})
			message(FATAL_ERROR "OUTPUT_DIR variable must not be absolute")
		endif ()

		#
		# Get the basename, use string REGEX REPLACE because
		# get_filename_component will remove all extensions while we only want
		# to remove md. (e.g. irccd.conf.md becomes irccd.conf).
		#
		get_filename_component(filename ${HTML_SOURCE} NAME)
		string(REGEX REPLACE "^(.*)\\.md$" "\\1" filename ${filename})

		# Compute baseurl.
		file(
			RELATIVE_PATH
			baseurl
			${CMAKE_BINARY_DIR}/html/${HTML_OUTPUT_DIR}
			${CMAKE_BINARY_DIR}/html
		)

		if (baseurl STREQUAL "")
			set(baseurl "./")
		endif ()
		if (NOT HTML_OUTPUT_DIR OR HTML_OUTPUT_DIR STREQUAL "")
			set(HTML_OUTPUT_DIR ".")
		endif ()

		# Filname path to output directory and files.
		set(outputdir ${CMAKE_BINARY_DIR}/html/${HTML_OUTPUT_DIR})
		set(output ${outputdir}/${filename}.html)

		# Build arguments.
		if (HTML_TEMPLATE)
			set(args -t ${HTML_TEMPLATE} -v baseurl="${baseurl}")
		endif ()

		add_custom_command(
			OUTPUT ${output}
			COMMAND
				${CMAKE_COMMAND} -E make_directory ${outputdir}
			COMMAND
				$<TARGET_FILE:marker::marker> ${args} ${HTML_VARIABLES}
				$<TARGET_FILE:marker::libmarker-bulma> ${output} ${HTML_SOURCE}
			DEPENDS
				${HTML_SOURCE}
				${HTML_TEMPLATE}
		)

		# Install the documentation file as component if provided.
		if (HTML_COMPONENT)
			install(
				FILES ${output}
				COMPONENT ${HTML_COMPONENT}
				DESTINATION ${CMAKE_INSTALL_DOCDIR}/${HTML_OUTPUT_DIR}
			)
		endif ()

		if (HTML_OUTPUT_VAR)
			set(${HTML_OUTPUT_VAR} ${output})
		endif ()
	endif ()
endmacro ()

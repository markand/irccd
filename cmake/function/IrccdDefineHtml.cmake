#
# IrccdDefineHtml.cmake -- CMake build system for irccd
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

#
# irccd_define_html
# -------------------------------------------------------------------
#
# irccd_define_html(
#	SOURCES the source files
#	DIRECTORY the base directory relative to the WITH_DOCDIR
#	TARGET the target name
#	VARS (Optional) variables to pass
# )
#
# This first signature processes all files and compile them one per one. Files are placed in the same hierarchy where
# the function is invoked.
#
# irccd_define_html(
#	SOURCES the source files
#	OUTPUT the output file
#	TARGET the target name
#	VARS (Optional) variables to pass
# )
#
# This second signature generated only one file from all sources. Usually for a book, guide and such.
#
# Note: do not pass an absolute path for the output.
#

function(irccd_define_html)
	set(options)
	set(oneValueArgs DIRECTORY OUTPUT TARGET)
	set(multiValueArgs ARGS SOURCES)

	cmake_parse_arguments(HTML "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	# Required arguments
	if (NOT HTML_SOURCES)
		message(FATAL_ERROR "Please specify source files")
	endif ()
	if (NOT HTML_TARGET)
		message(FATAL_ERROR "Please specify a target")
	endif ()

	set(base ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR})

	if (HTML_OUTPUT)
		#
		# First signature.
		#
		if (IS_ABSOLUTE)
			message(FATAL_ERROR "Do not pass absolute path for OUTPUT")
		endif ()

		get_filename_component(directory ${base}/${HTML_OUTPUT} DIRECTORY)
		file(RELATIVE_PATH baseurl ${directory} ${base})

		if (baseurl STREQUAL "")
			set(baseurl "./")
		endif ()

		# Configure sources
		foreach (s ${HTML_SOURCES})
			file(RELATIVE_PATH basepath ${CMAKE_CURRENT_SOURCE_DIR} ${s})
			configure_file(${s} ${CMAKE_CURRENT_BINARY_DIR}/${basepath} @ONLY)
			list(APPEND REAL_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${basepath})
		endforeach ()

		pandoc(
			OUTPUT ${base}/${HTML_OUTPUT}
			TARGET ${HTML_TARGET}
			SOURCES ${REAL_SOURCES}
			TEMPLATE ${resources_SOURCE_DIR}/template.html
			DEPENDS
				${resources_SOURCE_DIR}/template.html
				docs-resources
			ARGS ${HTML_ARGS}
			VARIABLE baseurl:${baseurl}
			FROM markdown TO html5
			STANTALONE MAKE_DIRECTORY
		)

		install(
			FILES ${base}/${HTML_OUTPUT}
			DESTINATION ${WITH_DOCDIR}
		)
	else ()
		if (NOT HTML_DIRECTORY)
			message(FATAL_ERROR "Please specify the directory")
		endif ()

		#
		# Second signature.
		#
		foreach (s ${HTML_SOURCES})
			file(RELATIVE_PATH input ${CMAKE_CURRENT_SOURCE_DIR} ${s})
			string(REGEX REPLACE "^(.*)\\.md$" "\\1.html" filename ${input})
			get_filename_component(directory ${base}/${HTML_DIRECTORY}/${filename} DIRECTORY)
			file(RELATIVE_PATH baseurl ${directory} ${base})
			file(RELATIVE_PATH basepath ${CMAKE_CURRENT_SOURCE_DIR} ${s})
			list(APPEND outputs ${base}/${HTML_DIRECTORY}/${filename})
			configure_file(${s} ${CMAKE_CURRENT_BINARY_DIR}/${basepath} @ONLY)

			pandoc(
				OUTPUT ${base}/${HTML_DIRECTORY}/${filename}
				SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${basepath}
				TEMPLATE ${resources_SOURCE_DIR}/template.html
				DEPENDS
					${resources_SOURCE_DIR}/template.html
					docs-resources
				ARGS ${HTML_ARGS}
				VARIABLE baseurl:${baseurl}
				FROM markdown TO html5
				STANTALONE MAKE_DIRECTORY
			)
		endforeach ()

		add_custom_target(
			${HTML_TARGET}
			DEPENDS ${outputs}
			SOURCES ${HTML_SOURCES}
		)

		install(
			DIRECTORY ${base}/${HTML_DIRECTORY}
			DESTINATION ${WITH_DOCDIR}
		)
	endif ()

	add_dependencies(docs ${HTML_TARGET})
endfunction()

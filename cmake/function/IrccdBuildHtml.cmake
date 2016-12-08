#
# IrccdBuildHtml.cmake -- CMake build system for irccd
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

include(CMakeParseArguments)

#
# irccd_build_html
# -------------------------------------------------------------------
#
# irccd_build_html(
#   COMPONENT (Optional) install the output documentation as the given component
#   OUTPUT (Optional) override output path
#   OUTPUT_VAR (Optional) store the output file in the output variable
#   SOURCE the source markdown file
# )
#
# Create a rule to build the markdown file specified by SOURCE parameter.
#
# By default, the output file is built in the same directory relative to the
# current project. Specifying /foo/bar/baz/foo.md as SOURCE from the
# directory /foo/var ends in an output file baz/foo.html.
#
# The output file path can be overriden with the OUTPUT variable which must be
# relative and must contain the filename without extension (e.g. api/misc/Foo).
#
# This macro does not create a target, just the output rule and the output file
# can be retrieved using the OUTPUT_VAR variable.
#
# Example:
#
# irccd_build_html(
#   COMPONENT documentation
#   OUTPUT dev/howto-create-a-plugin
#   SOURCE myfile.md
#   OUTPUT_VAR output
# )
#
# add_custom_target(mytarget DEPENDS ${output})
#
# It's perfectly safe to call this macro multiple times with the same COMPONENT.
#

macro(irccd_build_html)
    set(oneValueArgs COMPONENT OUTPUT OUTPUT_VAR SOURCE)

    cmake_parse_arguments(HTML "" "${oneValueArgs}" "" ${ARGN})

    if (NOT HTML_SOURCE)
        message(FATAL_ERROR "Missing SOURCE parameter")
    endif ()

    #
    # Example with SOURCES set to CMAKE_CURRENT_SOURCE_DIR/api/event/onMessage.md
    #
    # Extract the following variables:
    #
    # dirname:      api/event
    # basename:     onMessage
    # baseurl:      ../../
    #
    if (HTML_OUTPUT)
        if (IS_ABSOLUTE ${HTML_OUTPUT})
            message(FATAL_ERROR "OUTPUT variable must not be absolute")
        endif ()

        get_filename_component(dirname ${HTML_OUTPUT} DIRECTORY)
        get_filename_component(basename ${HTML_OUTPUT} NAME)
    else()
        get_filename_component(dirname ${HTML_SOURCE} DIRECTORY)
        file(RELATIVE_PATH dirname ${CMAKE_CURRENT_SOURCE_DIR} ${dirname})
        get_filename_component(basename ${HTML_SOURCE} NAME)
    endif ()

    # Remove extension from basename.
    string(REGEX REPLACE "^(.*)\\.md$" "\\1" basename ${basename})

    file(
            RELATIVE_PATH
            baseurl
            ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${dirname}
            ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}
    )

    if (baseurl STREQUAL "")
        set(baseurl "./")
    endif ()

    # Replace CMake variables.
    configure_file(
        ${HTML_SOURCE}
        ${CMAKE_CURRENT_BINARY_DIR}/${dirname}/${basename}.md
        @ONLY
    )

    # Create a list of parents for the breadcrumb widget.
    string(REPLACE "/" ";" parentlist "${dirname}")
    set(parents "  -\n")
    set(parents "${parents}    name: \"index\"\n")
    set(parents "${parents}    path: \"${baseurl}index.html\"\n")

    set(path "${baseurl}")
    foreach (p ${parentlist})
        set(path "${path}${p}/")
        set(parents "${parents}  -\n")
        set(parents "${parents}    name: \"${p}\"\n")
        set(parents "${parents}    path: \"${path}index.html\"\n")
    endforeach ()

    configure_file(
        ${html_SOURCE_DIR}/resources/metadata.yml
        ${CMAKE_CURRENT_BINARY_DIR}/${dirname}/${basename}.yml
    )

    # Pandoc the file.
    pandoc(
        OUTPUT ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${dirname}/${basename}.html
        SOURCES
            ${CMAKE_CURRENT_BINARY_DIR}/${dirname}/${basename}.yml
            ${CMAKE_CURRENT_BINARY_DIR}/${dirname}/${basename}.md
        DEPENDS ${HTML_SOURCE}
        TEMPLATE ${html_SOURCE_DIR}/resources/template.html
        FROM markdown
        TO html5
        STANDALONE TOC MAKE_DIRECTORY
    )

    # Install the documentation file as component if provided.
    if (HTML_COMPONENT)
        install(
            FILES ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${dirname}/${basename}.html
            COMPONENT ${HTML_COMPONENT}
            DESTINATION ${WITH_DOCDIR}/${dirname}
        )
    endif ()

    if (HTML_OUTPUT_VAR)
        set(${HTML_OUTPUT_VAR} ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${dirname}/${basename}.html)
    endif ()
endmacro ()

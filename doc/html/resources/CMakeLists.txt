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

project(resources)

#
# Install HTML resources
# -------------------------------------------------------------------
#

set(
    CSS_FILES
    css/bootstrap.min.css
    css/doc.css
    css/doc-guide.css
    css/tomorrow.css
)

set(
    JS_FILES
    js/bootstrap.min.js
    js/highlight.js
    js/jquery.min.js
)

foreach (f ${CSS_FILES} ${JS_FILES})
    get_filename_component(base ${f} DIRECTORY)
    get_filename_component(name ${f} NAME)
    list(APPEND OUTPUTS ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${base}/${name})
    list(APPEND SOURCES ${resources_SOURCE_DIR}/${f})
    list(APPEND COMMANDS COMMAND ${CMAKE_COMMAND} -E copy ${resources_SOURCE_DIR}/${f} ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${base})
    install(FILES ${resources_SOURCE_DIR}/${f} DESTINATION ${WITH_DOCDIR}/${base})
endforeach ()

add_custom_command(
    OUTPUT ${OUTPUTS}
    DEPENDS ${SOURCES}
    COMMENT "Copying HTML resources"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/css
    COMMAND ${CMAKE_COMMAND} -E make_directory ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/js
    ${COMMANDS}
)

add_custom_target(
    docs-resources
    SOURCES
        ${SOURCES}
        ${resources_SOURCE_DIR}/template.html
    DEPENDS ${OUTPUTS}
)

set_target_properties(
    docs-resources
    PROPERTIES
        PROJECT_LABEL resources
        FOLDER docs
)

add_dependencies(all-docs docs-resources)

#
# Copyright (c) 2013-2015 David Demelier <markand@malikania.fr>
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

project(doxygen)

set(DOXYGEN_INPUT ${CMAKE_SOURCE_DIR}/lib)
set(DOXYGEN_OUTPUT ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/doxygen)

if (NOT DOXYGEN_DOT_FOUND)
	set(DOXYGEN_HAVE_DOT "NO")
	set(DOXYGEN_DOT_PATH "")
else ()
	set(DOXYGEN_HAVE_DOT "YES")
	set(DOXYGEN_DOT_PATH "\"${DOXYGEN_DOT_EXECUTABLE}\"")
endif ()

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
	${CMAKE_BINARY_DIR}/Doxyfile
)

add_custom_target(
	docs-doxygen
	COMMENT "Generating doxygen documentation"
	COMMAND
		${CMAKE_COMMAND} -E make_directory ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/doxygen
	COMMAND
		${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY
		${CMAKE_SOURCE_DIR}
)

add_dependencies(all-docs docs-doxygen)

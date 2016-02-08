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
# ${DOC_OUTPUT}/css/bootstrap.css
# ${DOC_OUTPUT}/css/doc.css
# ${DOC_OUTPUT}/js/bootstrap.min.js
# ${DOC_OUTPUT}/js/jquery.min.js
#

set(
	FILES
	${resources_SOURCE_DIR}/css/bootstrap.css
	${resources_SOURCE_DIR}/css/doc.css
	${resources_SOURCE_DIR}/css/doc-guide.css
	${resources_SOURCE_DIR}/js/bootstrap.min.js
	${resources_SOURCE_DIR}/js/jquery.min.js
)

add_custom_command(
	OUTPUT
		${DOC_OUTPUT}/css/bootstrap.css
		${DOC_OUTPUT}/css/doc.css
		${DOC_OUTPUT}/css/doc-guide.css
		${DOC_OUTPUT}/js/bootstrap.min.js
		${DOC_OUTPUT}/js/jquery.min.js
	COMMAND
		${CMAKE_COMMAND} -E make_directory ${DOC_OUTPUT}/css
	COMMAND
		${CMAKE_COMMAND} -E copy ${resources_SOURCE_DIR}/css/bootstrap.css ${DOC_OUTPUT}/css/bootstrap.css
	COMMAND
		${CMAKE_COMMAND} -E copy ${resources_SOURCE_DIR}/css/doc.css ${DOC_OUTPUT}/css/doc.css
	COMMAND
		${CMAKE_COMMAND} -E copy ${resources_SOURCE_DIR}/css/doc-guide.css ${DOC_OUTPUT}/css/doc-guide.css
	COMMAND
		${CMAKE_COMMAND} -E copy ${resources_SOURCE_DIR}/js/bootstrap.min.js ${DOC_OUTPUT}/js/bootstrap.min.js
	COMMAND
		${CMAKE_COMMAND} -E copy ${resources_SOURCE_DIR}/js/jquery.min.js ${DOC_OUTPUT}/js/jquery.min.js
	DEPENDS ${FILES}
	COMMENT "Copying HTML resources..."
)

add_custom_target(
	docs-resources
	SOURCES
		${FILES}
		${resources_SOURCE_DIR}/template.html
	DEPENDS
		${DOC_OUTPUT}/css/bootstrap.css
		${DOC_OUTPUT}/css/doc.css
		${DOC_OUTPUT}/css/doc-guide.css
		${DOC_OUTPUT}/js/bootstrap.min.js
		${DOC_OUTPUT}/js/jquery.min.js
)

install(
	FILES
		${resources_SOURCE_DIR}/css/bootstrap.css
		${resources_SOURCE_DIR}/css/doc.css
		${resources_SOURCE_DIR}/css/doc-guide.css
	DESTINATION ${WITH_DOCDIR}/css
)

install(
	FILES
		${resources_SOURCE_DIR}/js/bootstrap.min.js
		${resources_SOURCE_DIR}/js/jquery.min.js
	DESTINATION ${WITH_DOCDIR}/js
)

add_dependencies(docs docs-resources)

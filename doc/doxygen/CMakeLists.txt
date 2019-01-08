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

project(doxygen)

set(DOXYGEN_FILE_PATTERNS "*.hpp")
set(DOXYGEN_PREDEFINED BOOST_ASIO_HAS_LOCAL_SOCKETS IRCCD_HAVE_SSL IRCCD_HAVE_JS)
set(DOXYGEN_PROJECT_NAME irccd)
set(DOXYGEN_PROJECT_NUMBER ${IRCCD_VERSION})
set(DOXYGEN_QUIET YES)
set(DOXYGEN_WARN_AS_ERROR YES)
set(DOXYGEN_WARN_NO_PARAMDOC YES)

set(DOXYGEN_STRIP_FROM_PATH
	libirccd/irccd
	libirccd-daemon/irccd
	libirccd-ctl/irccd
	libirccd-js/irccd
	libirccd-test/irccd
)

doxygen_add_docs(
	docs-doxygen
	${CMAKE_CURRENT_SOURCE_DIR}/mainpage.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/modules.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pages.cpp
	${CMAKE_SOURCE_DIR}/libirccd
	${CMAKE_SOURCE_DIR}/libirccd-daemon
	${CMAKE_SOURCE_DIR}/libirccd-ctl
	${CMAKE_SOURCE_DIR}/libirccd-js
	${CMAKE_SOURCE_DIR}/libirccd-test
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_dependencies(docs docs-doxygen)

#
# CMakeLists.txt -- CMake build system for irccd
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

if (IRCCD_HAVE_LIBEDIT)
	list(APPEND LIBRARIES ${Editline_LIBRARIES})
	list(APPEND INCLUDES ${Editline_INCLUDE_DIRS})
endif ()

if (IRCCD_HAVE_JS)
	list(APPEND LIBRARIES libirccd-js)
endif ()

irccd_define_executable(
	TARGET irccd-test
	LIBRARIES libirccd-test ${LIBRARIES}
	INCLUDES ${INCLUDES}
	DESCRIPTION "Plugin tester"
	SOURCES main.cpp
)

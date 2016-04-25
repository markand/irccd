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

project(doc)

# Meta targets `docs`
add_custom_target(
	docs ALL
	COMMENT "Building irccd documentation"
	SOURCES
		${doc_SOURCE_DIR}/procs/README.md
		${doc_SOURCE_DIR}/procs/00.tests.md
		${doc_SOURCE_DIR}/procs/00.new-release.md
		${doc_SOURCE_DIR}/procs/20.options.md
		${doc_SOURCE_DIR}/procs/30.options.md
		${doc_SOURCE_DIR}/procs/60.new-command.md
)

# Needed for fakeroot
if (WITH_DOXYGEN)
	add_subdirectory(doxygen)
endif ()

add_subdirectory(examples)
add_subdirectory(man)

if (WITH_HTML)
	add_subdirectory(html)
endif ()

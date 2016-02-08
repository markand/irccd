#
# CMakeLists.md -- CMake build system for irccd
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

set(
	FILE_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/index.md
	${CMAKE_CURRENT_LIST_DIR}/function/basename.md
	${CMAKE_CURRENT_LIST_DIR}/function/dirname.md
	${CMAKE_CURRENT_LIST_DIR}/function/exists.md
	${CMAKE_CURRENT_LIST_DIR}/function/remove.md
	${CMAKE_CURRENT_LIST_DIR}/function/stat.md
	${CMAKE_CURRENT_LIST_DIR}/method/basename.md
	${CMAKE_CURRENT_LIST_DIR}/method/close.md
	${CMAKE_CURRENT_LIST_DIR}/method/constructor.md
	${CMAKE_CURRENT_LIST_DIR}/method/dirname.md
	${CMAKE_CURRENT_LIST_DIR}/method/read.md
	${CMAKE_CURRENT_LIST_DIR}/method/readline.md
	${CMAKE_CURRENT_LIST_DIR}/method/remove.md
	${CMAKE_CURRENT_LIST_DIR}/method/seek.md
	${CMAKE_CURRENT_LIST_DIR}/method/stat.md
	${CMAKE_CURRENT_LIST_DIR}/method/tell.md
	${CMAKE_CURRENT_LIST_DIR}/method/write.md
)

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
	SERVER_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/index.md
	${CMAKE_CURRENT_LIST_DIR}/function/add.md
	${CMAKE_CURRENT_LIST_DIR}/function/find.md
	${CMAKE_CURRENT_LIST_DIR}/function/list.md
	${CMAKE_CURRENT_LIST_DIR}/function/remove.md
	${CMAKE_CURRENT_LIST_DIR}/method/cmode.md
	${CMAKE_CURRENT_LIST_DIR}/method/constructor.md
	${CMAKE_CURRENT_LIST_DIR}/method/cnotice.md
	${CMAKE_CURRENT_LIST_DIR}/method/info.md
	${CMAKE_CURRENT_LIST_DIR}/method/invite.md
	${CMAKE_CURRENT_LIST_DIR}/method/join.md
	${CMAKE_CURRENT_LIST_DIR}/method/kick.md
	${CMAKE_CURRENT_LIST_DIR}/method/me.md
	${CMAKE_CURRENT_LIST_DIR}/method/message.md
	${CMAKE_CURRENT_LIST_DIR}/method/mode.md
	${CMAKE_CURRENT_LIST_DIR}/method/names.md
	${CMAKE_CURRENT_LIST_DIR}/method/nick.md
	${CMAKE_CURRENT_LIST_DIR}/method/notice.md
	${CMAKE_CURRENT_LIST_DIR}/method/part.md
	${CMAKE_CURRENT_LIST_DIR}/method/topic.md
	${CMAKE_CURRENT_LIST_DIR}/method/whois.md
	${CMAKE_CURRENT_LIST_DIR}/method/toString.md
)

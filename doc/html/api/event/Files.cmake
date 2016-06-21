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
    EVENT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/onCommand.md
    ${CMAKE_CURRENT_LIST_DIR}/onConnect.md
    ${CMAKE_CURRENT_LIST_DIR}/onChannelMode.md
    ${CMAKE_CURRENT_LIST_DIR}/onChannelNotice.md
    ${CMAKE_CURRENT_LIST_DIR}/onInvite.md
    ${CMAKE_CURRENT_LIST_DIR}/onJoin.md
    ${CMAKE_CURRENT_LIST_DIR}/onKick.md
    ${CMAKE_CURRENT_LIST_DIR}/onLoad.md
    ${CMAKE_CURRENT_LIST_DIR}/onMessage.md
    ${CMAKE_CURRENT_LIST_DIR}/onMe.md
    ${CMAKE_CURRENT_LIST_DIR}/onMode.md
    ${CMAKE_CURRENT_LIST_DIR}/onNames.md
    ${CMAKE_CURRENT_LIST_DIR}/onNick.md
    ${CMAKE_CURRENT_LIST_DIR}/onNotice.md
    ${CMAKE_CURRENT_LIST_DIR}/onPart.md
    ${CMAKE_CURRENT_LIST_DIR}/onQuery.md
    ${CMAKE_CURRENT_LIST_DIR}/onQueryCommand.md
    ${CMAKE_CURRENT_LIST_DIR}/onReload.md
    ${CMAKE_CURRENT_LIST_DIR}/onTopic.md
    ${CMAKE_CURRENT_LIST_DIR}/onUnload.md
    ${CMAKE_CURRENT_LIST_DIR}/onWhois.md
)

#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

if (IRCCD_HAVE_JS)
    add_subdirectory(cli-plugin-config)
    add_subdirectory(cli-plugin-info)
    add_subdirectory(cli-plugin-list)
    add_subdirectory(cli-plugin-load)
    add_subdirectory(cli-plugin-reload)
    add_subdirectory(cli-plugin-unload)
endif ()

add_subdirectory(cli-rule-add)
add_subdirectory(cli-rule-edit)
add_subdirectory(cli-rule-info)
add_subdirectory(cli-rule-list)
add_subdirectory(cli-rule-move)
add_subdirectory(cli-rule-remove)

add_subdirectory(cli-server-disconnect)
add_subdirectory(cli-server-info)
add_subdirectory(cli-server-invite)
add_subdirectory(cli-server-join)
add_subdirectory(cli-server-kick)
add_subdirectory(cli-server-list)
add_subdirectory(cli-server-me)
add_subdirectory(cli-server-message)
add_subdirectory(cli-server-mode)
add_subdirectory(cli-server-nick)
add_subdirectory(cli-server-notice)
add_subdirectory(cli-server-part)
add_subdirectory(cli-server-reconnect)
add_subdirectory(cli-server-topic)

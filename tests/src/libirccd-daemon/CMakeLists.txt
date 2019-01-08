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

add_subdirectory(command-plugin-config)
add_subdirectory(command-plugin-info)
add_subdirectory(command-plugin-list)
add_subdirectory(command-plugin-load)
add_subdirectory(command-plugin-reload)
add_subdirectory(command-plugin-unload)
add_subdirectory(command-rule-add)
add_subdirectory(command-rule-edit)
add_subdirectory(command-rule-info)
add_subdirectory(command-rule-list)
add_subdirectory(command-rule-move)
add_subdirectory(command-rule-remove)
add_subdirectory(command-server-connect)
add_subdirectory(command-server-disconnect)
add_subdirectory(command-server-info)
add_subdirectory(command-server-invite)
add_subdirectory(command-server-join)
add_subdirectory(command-server-kick)
add_subdirectory(command-server-list)
add_subdirectory(command-server-me)
add_subdirectory(command-server-message)
add_subdirectory(command-server-mode)
add_subdirectory(command-server-nick)
add_subdirectory(command-server-notice)
add_subdirectory(command-server-part)
add_subdirectory(command-server-reconnect)
add_subdirectory(command-server-topic)

add_subdirectory(dynlib-plugin)
add_subdirectory(irc)
add_subdirectory(logger)
add_subdirectory(rules)
add_subdirectory(rule-util)
add_subdirectory(server)
add_subdirectory(server-util)

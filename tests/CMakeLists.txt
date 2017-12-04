#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

# Project
project(tests)

# Transport commands.
add_subdirectory(src/plugin-config-command)
add_subdirectory(src/plugin-info-command)
add_subdirectory(src/plugin-list-command)
add_subdirectory(src/plugin-load-command)
add_subdirectory(src/plugin-reload-command)
add_subdirectory(src/plugin-unload-command)
add_subdirectory(src/rule-add-command)
add_subdirectory(src/rule-edit-command)
add_subdirectory(src/rule-info-command)
add_subdirectory(src/rule-list-command)
add_subdirectory(src/rule-move-command)
add_subdirectory(src/rule-remove-command)
add_subdirectory(src/server-connect-command)
add_subdirectory(src/server-disconnect-command)
add_subdirectory(src/server-info-command)
add_subdirectory(src/server-invite-command)
add_subdirectory(src/server-join-command)
add_subdirectory(src/server-kick-command)
add_subdirectory(src/server-list-command)
add_subdirectory(src/server-me-command)
add_subdirectory(src/server-message-command)
add_subdirectory(src/server-mode-command)
add_subdirectory(src/server-nick-command)
add_subdirectory(src/server-notice-command)
add_subdirectory(src/server-part-command)
add_subdirectory(src/server-reconnect-command)
add_subdirectory(src/server-topic-command)

# Miscellaneous functions.
add_subdirectory(src/irc)
add_subdirectory(src/logger)
add_subdirectory(src/rules)
add_subdirectory(src/util)

# Javascript API and plugins.
if (HAVE_JS)
    add_subdirectory(src/js-plugin)

    add_subdirectory(src/elapsedtimer-jsapi)
    add_subdirectory(src/directory-jsapi)
    add_subdirectory(src/file-jsapi)
    add_subdirectory(src/irccd-jsapi)
    add_subdirectory(src/logger-jsapi)
    add_subdirectory(src/system-jsapi)
    add_subdirectory(src/timer-jsapi)
    add_subdirectory(src/unicode-jsapi)
    add_subdirectory(src/util-jsapi)

    add_subdirectory(src/plugin-ask)
    add_subdirectory(src/plugin-auth)
    add_subdirectory(src/plugin-hangman)
    add_subdirectory(src/plugin-history)
    add_subdirectory(src/plugin-logger)
    add_subdirectory(src/plugin-plugin)
endif ()

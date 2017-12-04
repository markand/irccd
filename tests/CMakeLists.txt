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

if (WITH_TESTS)
    add_subdirectory(plugin-config-command)
    add_subdirectory(plugin-info-command)
    add_subdirectory(plugin-list-command)
    add_subdirectory(plugin-load-command)
    add_subdirectory(plugin-reload-command)
    add_subdirectory(plugin-unload-command)
    add_subdirectory(rule-add-command)
    add_subdirectory(rule-edit-command)
    add_subdirectory(rule-info-command)
    add_subdirectory(rule-list-command)
    add_subdirectory(rule-move-command)
    add_subdirectory(rule-remove-command)
    add_subdirectory(server-connect-command)
    add_subdirectory(server-disconnect-command)
    add_subdirectory(server-info-command)
    add_subdirectory(server-invite-command)
    add_subdirectory(server-join-command)
    add_subdirectory(server-kick-command)
    add_subdirectory(server-list-command)
    add_subdirectory(server-me-command)
    add_subdirectory(server-message-command)
    add_subdirectory(server-mode-command)
    add_subdirectory(server-nick-command)
    add_subdirectory(server-notice-command)
    add_subdirectory(server-part-command)
    add_subdirectory(server-reconnect-command)
    add_subdirectory(server-topic-command)

    add_subdirectory(irc)
    add_subdirectory(logger)
    add_subdirectory(rules)
    add_subdirectory(util)

    if (HAVE_JS)
        add_subdirectory(js-plugin)

        add_subdirectory(elapsedtimer-jsapi)
        add_subdirectory(directory-jsapi)
        add_subdirectory(file-jsapi)
        add_subdirectory(irccd-jsapi)
        add_subdirectory(logger-jsapi)
        add_subdirectory(system-jsapi)
        add_subdirectory(timer-jsapi)
        add_subdirectory(unicode-jsapi)
        add_subdirectory(util-jsapi)

        add_subdirectory(plugin-ask)
        add_subdirectory(plugin-auth)
        add_subdirectory(plugin-hangman)
        add_subdirectory(plugin-history)
        add_subdirectory(plugin-logger)
        add_subdirectory(plugin-plugin)
    endif ()
endif ()

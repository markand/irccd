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
    # Create some hierarchy.
    file(MAKE_DIRECTORY ${tests_BINARY_DIR}/root)
    file(MAKE_DIRECTORY ${tests_BINARY_DIR}/root/level-a)
    file(MAKE_DIRECTORY ${tests_BINARY_DIR}/root/level-a/level-1)
    file(WRITE ${tests_BINARY_DIR}/root/file-1.txt "\n")
    file(WRITE ${tests_BINARY_DIR}/root/level-a/level-1/file-2.txt)

    add_subdirectory(cmd-plugin-config)
#    add_subdirectory(cmd-plugin-info)
#    add_subdirectory(cmd-plugin-list)
#    add_subdirectory(cmd-plugin-load)
#    add_subdirectory(cmd-plugin-reload)
#    add_subdirectory(cmd-plugin-unload)
    add_subdirectory(cmd-rule-add)
    add_subdirectory(cmd-rule-edit)
    add_subdirectory(cmd-rule-info)
    add_subdirectory(cmd-rule-list)
    add_subdirectory(cmd-rule-move)
    add_subdirectory(cmd-rule-remove)
    add_subdirectory(cmd-server-cmode)
    add_subdirectory(cmd-server-cnotice)
    add_subdirectory(cmd-server-connect)
    add_subdirectory(cmd-server-disconnect)
    add_subdirectory(cmd-server-info)
    add_subdirectory(cmd-server-invite)
    add_subdirectory(cmd-server-join)
    add_subdirectory(cmd-server-kick)
    add_subdirectory(cmd-server-list)
    add_subdirectory(cmd-server-me)
    add_subdirectory(cmd-server-message)
    add_subdirectory(cmd-server-mode)
    add_subdirectory(cmd-server-nick)
    add_subdirectory(cmd-server-notice)
    add_subdirectory(cmd-server-part)
    add_subdirectory(cmd-server-reconnect)
    add_subdirectory(cmd-server-topic)
#    add_subdirectory(dynlib_plugin)

    # Misc
    add_subdirectory(logger)
    add_subdirectory(rules)
    add_subdirectory(util)

    # Services
    #add_subdirectory(service-plugin)

    # JS API
    if (HAVE_JS)
        # Javascript plugin object.
        add_subdirectory(js-plugin)

        add_subdirectory(js)
        add_subdirectory(js-elapsedtimer)
        add_subdirectory(js-directory)
        add_subdirectory(js-file)
        add_subdirectory(js-irccd)
        add_subdirectory(js-logger)
        add_subdirectory(js-system)
        add_subdirectory(js-timer)
        add_subdirectory(js-unicode)
        add_subdirectory(js-util)
        add_subdirectory(plugin-ask)
        add_subdirectory(plugin-auth)
        #add_subdirectory(plugin-hangman)
        add_subdirectory(plugin-history)
        add_subdirectory(plugin-logger)
        add_subdirectory(plugin-plugin)
        add_subdirectory(timer)
    endif ()
endif ()

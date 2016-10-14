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

# Project
project(tests)

if (WITH_TESTS)
    add_subdirectory(cmd-server-cmode)
    add_subdirectory(cmd-server-cnotice)
    add_subdirectory(cmd-server-disconnect)
    add_subdirectory(cmd-server-me)
    add_subdirectory(cmd-server-message)
    add_subdirectory(cmd-server-mode)
    add_subdirectory(cmd-server-nick)
    add_subdirectory(cmd-server-notice)

#    # Misc
#    add_subdirectory(command)
#    add_subdirectory(elapsedtimer)
#    add_subdirectory(logger)
#    add_subdirectory(path)
#    add_subdirectory(rules)
#    add_subdirectory(timer)
#    add_subdirectory(util)
#
#    # JS API
#    if (WITH_JS)
#        add_subdirectory(js-elapsedtimer)
#        add_subdirectory(js-file)
#        add_subdirectory(js-irccd)
#        add_subdirectory(js-logger)
#        add_subdirectory(js-system)
#        add_subdirectory(js-timer)
#        add_subdirectory(js-unicode)
#        add_subdirectory(js-util)
#        add_subdirectory(plugin-ask)
#        add_subdirectory(plugin-auth)
#        add_subdirectory(plugin-hangman)
#        add_subdirectory(plugin-history)
#        add_subdirectory(plugin-logger)
#        add_subdirectory(plugin-plugin)
#    endif ()
endif ()

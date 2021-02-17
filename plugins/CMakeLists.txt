#
# CMakeLists.txt -- CMake build for irccd
#
# Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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
	JS_PLUGINS
	ask
	auth
	hangman
	history
	joke
	logger
	plugin
	roulette
	tictactoe
)

set(
	C_PLUGINS
	links
)

foreach (p ${C_PLUGINS} ${JS_PLUGINS})
	string(TOUPPER ${p} opt)
	option(IRCCD_WITH_PLUGIN_${opt} "Enable ${p} plugin" On)
endforeach ()

if (IRCCD_WITH_JS)
	foreach (p ${JS_PLUGINS})
		string(TOUPPER ${p} p)
		if (IRCCD_WITH_PLUGIN_${p})
			add_subdirectory(${p})
		endif ()
	endforeach ()
endif ()

foreach (p ${C_PLUGINS})
	string(TOUPPER ${p} p)

	if (IRCCD_WITH_PLUGIN_${p})
		add_subdirectory(links)
	endif ()
endforeach ()

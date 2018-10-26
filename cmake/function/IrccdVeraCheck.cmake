#
# IrccdVeraCheck.cmake -- CMake build system for irccd
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

#
# irccd_vera_check
# ----------------
#
# irccd_vera_check(target sources)
#
# Check the style source code using vera++.
#
# No-op if IRCCD_HAVE_VERA is Off.
#
# This macro add a post-build command to call vera++ program on the specified
# sources file for the given target.
#

function(irccd_vera_check target sources)
	if (IRCCD_HAVE_VERA)
		set(valid ".cpp;.c;.hpp;.h")

		# Cleanup non relevant files.
		foreach (s ${sources})
			get_filename_component(s ${s} ABSOLUTE)
			get_filename_component(ext ${s} EXT)

			foreach (e ${valid})
				if (${ext} STREQUAL ${e})
					list(APPEND newsources ${s})
				endif ()
			endforeach ()
		endforeach ()

		add_custom_command(
			TARGET ${target}
			COMMAND
				$<TARGET_FILE:vera> -w --root ${CMAKE_SOURCE_DIR}/vera ${newsources}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			VERBATIM
		)
	endif ()
endfunction()

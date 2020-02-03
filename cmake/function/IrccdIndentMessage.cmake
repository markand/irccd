#
# IrccdIndentMessage.cmake -- CMake build system for irccd
#
# Copyright (c) 2016-2020 David Demelier <markand@malikania.fr>
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
# irccd_indent_message
# --------------------
#
# irccd_indent_message(variable value padding)
#
# Indent a message and its value with the specified amount of spaces between
# variable and value.
#
# Example:
#
# irccd_indent_message("godmode: " "false", 20)
#
# Will output:
#
# godmode:            false
#

function(irccd_indent_message var value padding)
	string(LENGTH "${var}" length)

	while (${length} LESS ${padding})
		math(EXPR length "${length} + 1")
		set(space "${space} ")
	endwhile ()

	message("${var}${space}${value}")
endfunction()

#
# IrccdDefineMan.cmake -- CMake build system for irccd
#
# Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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
# irccd_define_man
# ----------------
#
# Synopsis:
#
# irccd_define_man(
#   INPUT       file path to the input man page
#   SECTION     section (e.g. man1 man3)
#   OUTPUT      (Optional) file name to rename
# )
#
# This function configure the manual and install it if IRCCD_WITH_MAN is set.
#

option(IRCCD_WITH_MAN "Install man pages" On)

function(irccd_define_man)
	set(options "")
	set(oneValueArgs "INPUT;OUTPUT;SECTION")
	set(multiValueArgs "")

	cmake_parse_arguments(MAN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT MAN_INPUT)
		message(FATAL_ERROR "Argument INPUT required")
	endif ()
	if (NOT MAN_SECTION)
		message(FATAL_ERROR "Argument SECTION required")
	endif ()

	if (NOT MAN_OUTPUT)
		get_filename_component(output ${MAN_INPUT} NAME)
	else ()
		set(output ${MAN_OUTPUT})
	endif ()

	if (IRCCD_WITH_MAN)
		configure_file(
			${MAN_INPUT}
			${CMAKE_BINARY_DIR}/man/${output}
			@ONLY
		)
		install(
			FILES ${CMAKE_BINARY_DIR}/man/${output}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/${MAN_SECTION}
		)
	endif ()
endfunction()

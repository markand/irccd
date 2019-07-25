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

project(systemd)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(DEFAULT_SYSTEMD "Yes")
else ()
	set(DEFAULT_SYSTEMD "No")
endif ()

option(IRCCD_WITH_SYSTEMD "Install systemd service" ${DEFAULT_SYSTEMD})

if (IRCCD_WITH_SYSTEMD)
	irccd_set_global(IRCCD_WITH_SYSTEMD_MSG "Yes")

	set(PATH "${CMAKE_INSTALL_FULL_BINDIR}/irccd")

	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/irccd.service
		${CMAKE_CURRENT_BINARY_DIR}/irccd.service
	)

	install(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/irccd.service
		DESTINATION "/usr/lib/systemd/system"
	)
else ()
	irccd_set_global(IRCCD_WITH_SYSTEMD_MSG "No (disabled by user)")
endif()

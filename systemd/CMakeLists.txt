#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
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

find_program(SYSTEMCTL_EXECUTABLE systemctl)

if (SYSTEMCTL_EXECUTABLE)
	set(default On)
else ()
	set(default Off)
endif ()

option(IRCCD_WITH_SYSTEMD "Install irccd.service" ${default})

if (IRCCD_WITH_SYSTEMD)
	set(PATH ${CMAKE_INSTALL_FULL_BINDIR}/irccd)

	configure_file(
		${systemd_SOURCE_DIR}/irccd.service.in
		${systemd_BINARY_DIR}/irccd.service
	)

	install(
		FILES ${systemd_BINARY_DIR}/irccd.service
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system
	)
endif ()

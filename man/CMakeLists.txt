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

project(man)

function(man file section)
	get_filename_component(basename ${file} NAME)
	configure_file(${file} ${man_BINARY_DIR}/${basename})
	install(
		FILES ${man_BINARY_DIR}/${basename}
		DESTINATION ${CMAKE_INSTALL_MANDIR}/${section}
	)
endfunction()

set(
	MAN1
	${man_SOURCE_DIR}/irccd.1
	${man_SOURCE_DIR}/irccdctl.1
)

set(
	MAN3
)

if (IRCCD_WITH_JS)
	list(
		APPEND MAN3
		${man_SOURCE_DIR}/irccd-api-chrono.3
		${man_SOURCE_DIR}/irccd-api-directory.3
		${man_SOURCE_DIR}/irccd-api-file.3
		${man_SOURCE_DIR}/irccd-api-hook.3
		${man_SOURCE_DIR}/irccd-api-logger.3
		${man_SOURCE_DIR}/irccd-api-plugin.3
		${man_SOURCE_DIR}/irccd-api-rule.3
		${man_SOURCE_DIR}/irccd-api-server.3
		${man_SOURCE_DIR}/irccd-api-system.3
		${man_SOURCE_DIR}/irccd-api-timer.3
		${man_SOURCE_DIR}/irccd-api-unicode.3
		${man_SOURCE_DIR}/irccd-api-util.3
		${man_SOURCE_DIR}/irccd-api.3
	)
endif ()

set(
	MAN5
	${man_SOURCE_DIR}/irccd.conf.5
)

set(
	MAN7
	${man_SOURCE_DIR}/irccd-ipc.7
	${man_SOURCE_DIR}/irccd-templates.7
)

foreach (m ${MAN1})
	man(${m} man1)
endforeach ()

foreach (m ${MAN3})
	man(${m} man3)
endforeach ()

foreach (m ${MAN5})
	man(${m} man5)
endforeach ()

foreach (m ${MAN7})
	man(${m} man7)
endforeach ()

#
# CMakeLists.txt -- CMake build system for irccd
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

set(
	MANPAGES_1
	${man_SOURCE_DIR}/irccdctl.1
	${man_SOURCE_DIR}/irccd.1
)

set(
	MANPAGES_3
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
	${man_SOURCE_DIR}/libirccd-channel.3
	${man_SOURCE_DIR}/libirccd-event.3
	${man_SOURCE_DIR}/libirccd-hook.3
	${man_SOURCE_DIR}/libirccd-log.3
	${man_SOURCE_DIR}/libirccd-rule.3
	${man_SOURCE_DIR}/libirccd-server.3
	${man_SOURCE_DIR}/libirccd-subst.3
	${man_SOURCE_DIR}/libirccd-util.3
	${man_SOURCE_DIR}/libirccd.3
)

set(
	MANPAGES_5
	${man_SOURCE_DIR}/irccd.conf.5
)

set(
	MANPAGES_7
	${man_SOURCE_DIR}/irccd-ipc.7
	${man_SOURCE_DIR}/irccd-templates.7
)

add_custom_target(
	man
	SOURCES
		${MANPAGES_1}
		${MANPAGES_3}
		${MANPAGES_5}
		${MANPAGES_7}
)

install(FILES ${MANPAGES_1} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(FILES ${MANPAGES_3} DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${MANPAGES_5} DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)
install(FILES ${MANPAGES_7} DESTINATION ${CMAKE_INSTALL_MANDIR}/man7)

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

project(examples)

add_custom_command(
	OUTPUT
		${IRCCD_FAKEROOTDIR}/${WITH_CONFDIR}/irccd.conf.sample
		${IRCCD_FAKEROOTDIR}/${WITH_CONFDIR}/irccdctl.conf.sample
	COMMAND
		${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/irccd.conf ${IRCCD_FAKEROOTDIR}/${WITH_CONFDIR}/irccd.conf.sample
	COMMAND
		${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/irccdctl.conf ${IRCCD_FAKEROOTDIR}/${WITH_CONFDIR}/irccdctl.conf.sample
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/irccd.conf
		${CMAKE_CURRENT_SOURCE_DIR}/irccdctl.conf
)

add_custom_target(
	docs-examples
	DEPENDS
		${IRCCD_FAKEROOTDIR}/${WITH_CONFDIR}/irccd.conf.sample
		${IRCCD_FAKEROOTDIR}/${WITH_CONFDIR}/irccdctl.conf.sample
	SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/irccd.conf
		${CMAKE_CURRENT_SOURCE_DIR}/irccdctl.conf
)

add_dependencies(docs docs-examples)

install(
	FILES irccdctl.conf
	DESTINATION "${WITH_CONFDIR}"
	RENAME "irccdctl.conf.sample"
)

install(
	FILES irccd.conf
	DESTINATION "${WITH_CONFDIR}"
	RENAME "irccd.conf.sample"
)

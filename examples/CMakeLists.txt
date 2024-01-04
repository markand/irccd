#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2024 David Demelier <markand@malikania.fr>
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

set(
	CONFIGS
	${examples_SOURCE_DIR}/irccd.conf.sample
)

set(
	PLUGINS
	${examples_SOURCE_DIR}/sample-plugin.c
	${examples_SOURCE_DIR}/sample-plugin.js
	${examples_SOURCE_DIR}/sample-hook.sh
)

add_custom_target(doc SOURCES ${CONFIGS} ${PLUGINS})
install(FILES ${CONFIGS} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
install(FILES ${PLUGINS} DESTINATION ${CMAKE_INSTALL_DOCDIR})
source_group("" FILES ${CONFIGS} ${PLUGINS})

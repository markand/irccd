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

project(guide)

include(01-intro/Files.cmake)
include(02-install/Files.cmake)
include(03-config-format/Files.cmake)
include(04-irccd/Files.cmake)
include(05-irccdctl/Files.cmake)
include(06-plugin/Files.cmake)
include(07-socket/Files.cmake)

set(
	SOURCES
	${INTRO_SOURCES}
	${INSTALL_SOURCES}
	${CONFIG_FORMAT_SOURCES}
	${IRCCD_SOURCES}
	${IRCCDCTL_SOURCES}
	${PLUGIN_SOURCES}
	${SOCKET_SOURCES}
)

irccd_define_html(
	OUTPUT guide.html
	TARGET docs-guide
	SOURCES ${SOURCES}
	ARGS --toc --toc-depth=2 -Vguide
)

set_target_properties(
	docs-guide
	PROPERTIES
		PROJECT_LABEL guide
		FOLDER docs
)
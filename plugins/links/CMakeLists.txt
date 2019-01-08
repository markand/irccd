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

project(plugin-links)

find_package(Uriparser)

if (NOT Uriparser_FOUND)
	setg(IRCCD_WITH_PLUGIN_LINKS_MSG "No (uriparser not found)")
else ()
	irccd_define_plugin(
		NAME links
		TYPE NATIVE
		SOURCES
			${plugin-links_SOURCE_DIR}/links.cpp
			${plugin-links_SOURCE_DIR}/links.hpp
			${plugin-links_SOURCE_DIR}/requester.cpp
			${plugin-links_SOURCE_DIR}/requester.hpp
			${plugin-links_SOURCE_DIR}/scope_exit.cpp
			${plugin-links_SOURCE_DIR}/scope_exit.hpp
			${plugin-links_SOURCE_DIR}/uri.cpp
			${plugin-links_SOURCE_DIR}/uri.hpp
		INCLUDES ${Uriparser_INCLUDE_DIRS}
		LIBRARIES ${Uriparser_LIBRARIES}
	)
endif ()

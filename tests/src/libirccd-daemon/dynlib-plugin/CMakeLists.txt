#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

find_package(Boost REQUIRED QUIET)

add_library(sample-plugin MODULE sample_plugin.cpp)
target_link_libraries(sample-plugin libirccd-test Boost::boost)

set_target_properties(
	sample-plugin
	PROPERTIES
		PREFIX ""
		FOLDER "test"
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

foreach (cfg ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${cfg} cfg)
	set_target_properties(
		sample-plugin
		PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_CURRENT_BINARY_DIR}
			LIBRARY_OUTPUT_DIRECTORY_${cfg} ${CMAKE_CURRENT_BINARY_DIR}
	)
endforeach ()

irccd_define_test(
	NAME dynlib-plugin
	SOURCES main.cpp
	LIBRARIES libirccd libirccd-test
	DEPENDS sample-plugin
)

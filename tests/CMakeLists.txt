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

project(tests)

set(
	TESTS
	bot
	channel
	dl-plugin
	event
	jsapi-chrono
	jsapi-directory
	jsapi-file
	jsapi-irccd
	jsapi-system
	jsapi-timer
	jsapi-unicode
	jsapi-util
	log
	plugin-ask
	plugin-auth
	plugin-hangman
	plugin-history
	plugin-joke
	plugin-logger
	plugin-plugin
	plugin-tictactoe
	rule
	subst
	util
)

foreach (t ${TESTS})
	add_executable(test-${t} ${tests_SOURCE_DIR}/test-${t}.c)
	add_test(NAME test-${t} COMMAND test-${t})
	set_target_properties(test-${t} PROPERTIES FOLDER tests ENABLE_EXPORTS On)
	target_link_libraries(test-${t} irccd-static libirccd-greatest)
	target_compile_definitions(test-${t} PRIVATE TOP="${CMAKE_SOURCE_DIR}")
endforeach ()

#
# Test: dl-plugin
#
# We need a sample plugin to open.
#
add_library(plugin-dl-example MODULE ${tests_SOURCE_DIR}/data/example-dl-plugin.c)
add_dependencies(test-dl-plugin plugin-dl-example)
set_target_properties(plugin-dl-example PROPERTIES PREFIX "" FOLDER tests)
target_compile_definitions(test-dl-plugin PRIVATE EXAMPLE_DL_PLUGIN="$<TARGET_FILE:plugin-dl-example>")
target_link_libraries(plugin-dl-example irccd)

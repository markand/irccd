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

project(tests)

set(
	TESTS
	test-bot
	test-channel
	test-event
	test-dl-plugin
	test-log
	test-rule
	test-subst
	test-util
)

if (IRCCD_WITH_JS)
	list(
		APPEND TESTS
		test-jsapi-chrono
		test-jsapi-directory
		test-jsapi-file
		test-jsapi-irccd
		test-jsapi-system
		test-jsapi-timer
		test-jsapi-unicode
		test-jsapi-util
		test-plugin-ask
		test-plugin-auth
		test-plugin-hangman
		test-plugin-history
		test-plugin-joke
		test-plugin-logger
		test-plugin-plugin
		test-plugin-tictactoe
	)
endif ()

add_library(example-dl-plugin MODULE ${tests_SOURCE_DIR}/data/example-dl-plugin.c)
target_link_libraries(example-dl-plugin libirccd)

if (APPLE)
	set_target_properties(example-dl-plugin PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif ()

set_target_properties(example-dl-plugin
	PROPERTIES
		FOLDER tests/test-dl-plugin
		PREFIX ""
)

foreach (t ${TESTS})
	add_executable(${t} ${t}.c)
	add_test(${t} ${t})
	target_link_libraries(${t} irccd-fe libirccd-greatest)
	set_target_properties(${t} PROPERTIES FOLDER "tests")
	target_compile_definitions(
		${t}
		PRIVATE
			EXAMPLE_DL_PLUGIN="$<TARGET_FILE:example-dl-plugin>"
			IRCCD_EXECUTABLE="$<TARGET_FILE:irccd>"
			CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
			# TODO: change those names.
			BINARY="${tests_BINARY_DIR}"
			SOURCE="${tests_SOURCE_DIR}"
	)
	add_dependencies(${t} irccd example-dl-plugin)
endforeach ()

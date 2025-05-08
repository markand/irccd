#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

# TODO: meh, CMake needs top-level this.
find_package(CURL REQUIRED)
find_package(Ev REQUIRED)

set(
	TESTS
	bot
	channel
	dl-plugin
	event
	log
	rule
	subst
	util
	jsapi-chrono
	jsapi-directory
	jsapi-file
	jsapi-irccd
	jsapi-system
	jsapi-timer
	jsapi-unicode
	jsapi-util
)

foreach (t ${TESTS})
	add_executable(test-${t} ${tests_SOURCE_DIR}/test-${t}.c)
	add_test(NAME test-${t} COMMAND test-${t})
	set_target_properties(test-${t} PROPERTIES FOLDER tests ENABLE_EXPORTS On)
	target_link_libraries(test-${t} irccd-static libirccd-greatest)
	target_compile_definitions(
		test-${t}
		PRIVATE
			TOP="${CMAKE_SOURCE_DIR}"
			IRCCD_EXECUTABLE="$<TARGET_FILE:irccd>"
	)
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

#
# plugins are based on a mocked server, we rebuild most of the library
# and irccd using its fake symbols.
#
set(
	SOURCES
	${CMAKE_SOURCE_DIR}/extern/libbsd/reallocarray.c
	${CMAKE_SOURCE_DIR}/extern/libbsd/strlcat.c
	${CMAKE_SOURCE_DIR}/extern/libbsd/strlcpy.c
	${CMAKE_SOURCE_DIR}/irccd/js-plugin.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-chrono.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-directory.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-file.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-hook.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-http.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-irccd.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-logger.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-plugin.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-rule.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-server.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-system.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-timer.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-unicode.c
	${CMAKE_SOURCE_DIR}/irccd/jsapi-util.c
	${CMAKE_SOURCE_DIR}/irccd/unicode.c
	${CMAKE_SOURCE_DIR}/lib/irccd/hook.c
	${CMAKE_SOURCE_DIR}/lib/irccd/irccd.c
	${CMAKE_SOURCE_DIR}/lib/irccd/log.c
	${CMAKE_SOURCE_DIR}/lib/irccd/plugin.c
	${CMAKE_SOURCE_DIR}/lib/irccd/rule.c
	${CMAKE_SOURCE_DIR}/lib/irccd/subst.c
	${CMAKE_SOURCE_DIR}/lib/irccd/util.c
)

set(
	PLUGINS
	plugin-ask
	plugin-auth
	plugin-hangman
	plugin-history
	plugin-joke
	plugin-logger
	plugin-plugin
	plugin-tictactoe
)

foreach (t ${PLUGINS})
	add_executable(test-${t} ${tests_SOURCE_DIR}/test-${t}.c ${SOURCES})
	add_test(NAME test-${t} COMMAND test-${t})
	set_target_properties(test-${t} PROPERTIES FOLDER tests)
	target_link_libraries(test-${t}
		libirccd-greatest
		libirccd-utlist
		libirccd-duktape
		CURL::libcurl
		Ev::Ev
	)
	target_include_directories(
		test-${t}
		PRIVATE
			${CMAKE_BINARY_DIR}/lib
			${CMAKE_BINARY_DIR}/lib/irccd
			${CMAKE_SOURCE_DIR}
			${CMAKE_SOURCE_DIR}/irccd
			${CMAKE_SOURCE_DIR}/lib
	)
	target_compile_definitions(
		test-${t}
		PRIVATE
			TOP="${CMAKE_SOURCE_DIR}"
	)
endforeach ()

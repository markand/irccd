#
# IrccdDefineTest.cmake -- CMake build system for irccd
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

#
# irccd_define_test
# -------------------------------------------------------------------
#
# irccd_define_test(
#	NAME the test name
#	SOURCES the sources files
#	LIBRARIES (Optional) libraries to link
#	RESOURCES (Optional) some resources file to copy
# )
#
# Create a unit test named test-${NAME}
#
# Resources files are copied VERBATIM into the same directory.
#

function(irccd_define_test)
	set(oneValueArgs NAME)
	set(multiValueArgs SOURCES LIBRARIES RESOURCES)

	cmake_parse_arguments(TEST "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT TEST_NAME)
		message(FATAL_ERROR "Please set NAME")
	endif ()
	if (NOT TEST_SOURCES)
		message(FATAL_ERROR "Please set SOURCES")
	endif ()

	foreach (r ${TEST_RESOURCES})
		file(RELATIVE_PATH output ${CMAKE_CURRENT_SOURCE_DIR} ${r})
	
		add_custom_command(
			OUTPUT ${CMAKE_BINARY_DIR}/tests/${output}
			COMMAND ${CMAKE_COMMAND} -E copy ${r} ${CMAKE_BINARY_DIR}/tests/${output}
			DEPENDS ${r}
		)

		list(APPEND RESOURCES ${CMAKE_BINARY_DIR}/tests/${output})
	endforeach ()

	# Always link to googletest
	list(APPEND TEST_LIBRARIES extern-gtest)

	# Executable
	add_executable(test-${TEST_NAME} ${TEST_SOURCES} ${TEST_RESOURCES} ${RESOURCES})
	target_link_libraries(test-${TEST_NAME} ${TEST_LIBRARIES})
	source_group(Auto-generated FILES ${RESOURCES})

	target_include_directories(
		test-${TEST_NAME}
		PRIVATE
			${irccd_SOURCE_DIR}
	)

	target_compile_definitions(
		test-${TEST_NAME}
		PRIVATE
			IRCCD_TESTS_DIRECTORY="${CMAKE_BINARY_DIR}/tests"
	)

	# Tests are all in the same directory
	set_target_properties(
		test-${TEST_NAME}
		PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
			RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/tests
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/tests
			RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/tests
			RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/tests
	)

	if (UNIX)
		set_target_properties(test-${TEST_NAME} PROPERTIES LINK_FLAGS -pthread)
	endif ()

	# And test
	add_test(
		NAME test-${TEST_NAME}
		COMMAND test-${TEST_NAME}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
	)
endfunction()

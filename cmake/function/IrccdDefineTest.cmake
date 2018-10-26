#
# IrccdDefineTest.cmake -- CMake build system for irccd
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

#
# irccd_define_test
# -----------------
#
# irccd_define_test(
#   NAME        the test name
#   SOURCES     the sources files
#   LIBRARIES   (Optional) libraries to link
#   FLAGS       (Optional) compilation flags
#   DEPENDS     (Optional) list of dependencies
# )
#
# Create a unit test named test-${NAME}
#
# Resources files are copied VERBATIM into the same directory.
#

find_package(Boost REQUIRED QUIET COMPONENTS unit_test_framework)

include(${CMAKE_CURRENT_LIST_DIR}/IrccdVeraCheck.cmake)

function(irccd_define_test)
	set(oneValueArgs NAME)
	set(multiValueArgs DEPENDS SOURCES LIBRARIES FLAGS)

	cmake_parse_arguments(TEST "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT TEST_NAME)
		message(FATAL_ERROR "Please set NAME")
	endif ()
	if (NOT TEST_SOURCES)
		message(FATAL_ERROR "Please set SOURCES")
	endif ()

	list(
		APPEND
		TEST_LIBRARIES
			libirccd-test
			${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	)

	add_executable(test-${TEST_NAME} ${TEST_SOURCES})

	if (TEST_DEPENDS)
		add_dependencies(test-${TEST_NAME} ${TEST_DEPENDS})
	endif ()

	target_link_libraries(test-${TEST_NAME} ${TEST_LIBRARIES})

	target_include_directories(
		test-${TEST_NAME}
		PRIVATE
			${irccd_SOURCE_DIR}
	)

	target_compile_definitions(
		test-${TEST_NAME}
		PRIVATE
			${TEST_FLAGS}
			BOOST_TEST_DYN_LINK
			TESTS_SOURCE_DIR="${tests_SOURCE_DIR}"
			TESTS_BINARY_DIR="${tests_SOURCE_DIR}"
			CMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
			CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
			CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
			CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
	)

	# Tests are all in the same directory
	set_target_properties(
		test-${TEST_NAME}
		PROPERTIES
			PROJECT_LABEL ${TEST_NAME}
			FOLDER test
			RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	)
	foreach (c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${c} cu)
		set_target_properties(
			test-${TEST_NAME}
			PROPERTIES
				RUNTIME_OUTPUT_DIRECTORY_${cu} ${CMAKE_BINARY_DIR}/bin/${c}
		)
	endforeach()

	# And test
	add_test(
		NAME test-${TEST_NAME}
		COMMAND test-${TEST_NAME}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
	)

	irccd_vera_check(test-${TEST_NAME} "${TEST_SOURCES}")
endfunction()

#
# IrccdDefineExecutable.cmake -- CMake build system for irccd
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
# irccd_define_executable
# -------------------------------------------------------------------
#
# irccd_define_executable(
#	TARGET target name
#	SOURCES src1, src2, srcn
#	FLAGS (Optional) C/C++ flags (without -D)
#	LIBRARIES (Optional) libraries to link
#	INCLUDES (Optional) includes for the target
#	INSTALL (Optional) if set, install the executable (default: false)
#	PRIVATE (Optional) if set, do not build it into the fake root (default: false)
# )
#
# Create an executable that can be installed or not.
#

function(irccd_define_executable)
	set(options INSTALL PRIVATE)
	set(oneValueArgs TARGET)
	set(multiValueArgs SOURCES FLAGS LIBRARIES INCLUDES)

	cmake_parse_arguments(EXE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT EXE_TARGET)
		message(FATAL_ERROR "Please set TARGET")
	endif ()
	if (NOT EXE_SOURCES)
		message(FATAL_ERROR "Please set SOURCES")
	endif ()

	if (EXE_INSTALL AND EXE_PRIVATE)
		message(FATAL_ERROR "INSTALL and PRIVATE are mutually exclusive")
	endif ()

	add_executable(${EXE_TARGET} ${EXE_SOURCES})
	target_include_directories(${EXE_TARGET} PRIVATE ${EXE_INCLUDES})
	target_compile_definitions(${EXE_TARGET} PRIVATE ${EXE_FLAGS})
	target_link_libraries(${EXE_TARGET} ${EXE_LIBRARIES})

	# use fakeroot if relocatable for public executables.
	if (IRCCD_RELOCATABLE AND NOT EXE_PRIVATE)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/fakeroot/${WITH_BINDIR})

		set_target_properties(
			${EXE_TARGET}
			PROPERTIES
				RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/fakeroot/${WITH_BINDIR}
				RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/fakeroot/${WITH_BINDIR}
				RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/fakeroot/${WITH_BINDIR}
		)
	endif ()

	# Install the target.
	if (EXE_INSTALL)
		install(
			TARGETS ${EXE_TARGET}
			RUNTIME DESTINATION ${WITH_BINDIR}
		)
	endif ()
endfunction()

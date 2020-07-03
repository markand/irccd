#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2016-2020 David Demelier <markand@malikania.fr>
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
# irccd_install_dependencies(target)
# ----------------------------------
#
# Create an install rule to add runtime dependencies for the given target.
#
# This function is no-op on non Windows systems.
#

include(GNUInstallDirs)

set(IRCCD_MID_FILE ${CMAKE_CURRENT_LIST_DIR}/dependencies.cmake.in)

function(irccd_install_dependencies target)
	if (CMAKE_SYSTEM_NAME MATCHES "Windows")
		set(TARGET ${target})
		set(BINDIR ${CMAKE_INSTALL_BINDIR})

		# Change TARGET and PREFIX in dependencies.cmake
		configure_file(
			${IRCCD_MID_FILE}
			${CMAKE_CURRENT_BINARY_DIR}/dependencies.cmake.in
			@ONLY
		)

		# Generate a file with target file name changed.
		file(
			GENERATE
			OUTPUT
				${CMAKE_CURRENT_BINARY_DIR}/dependencies-$<CONFIG>.cmake
			INPUT
				${CMAKE_CURRENT_BINARY_DIR}/dependencies.cmake.in
		)

		# Execute installation of dependencies at install step.
		install(
			CODE "include(\"${CMAKE_CURRENT_BINARY_DIR}/dependencies-\${CMAKE_INSTALL_CONFIG_NAME\}.cmake\")\n"
			COMPONENT Core
		)
	endif ()
endfunction()

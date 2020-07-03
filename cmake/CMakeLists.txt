#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

project(cmake)

install(
	FILES
		${cmake_SOURCE_DIR}/function/IrccdSetGlobal.cmake
		${cmake_SOURCE_DIR}/function/IrccdDefineExecutable.cmake
		${cmake_SOURCE_DIR}/function/IrccdDefineLibrary.cmake
		${cmake_SOURCE_DIR}/function/IrccdDefineMan.cmake
		${cmake_SOURCE_DIR}/function/IrccdDefinePlugin.cmake
		${cmake_SOURCE_DIR}/function/IrccdIndentMessage.cmake
		${cmake_SOURCE_DIR}/function/IrccdInstallDependencies.cmake
		${cmake_SOURCE_DIR}/function/dependencies.cmake.in
	DESTINATION
		${CMAKE_INSTALL_LIBDIR}/cmake/irccd
)

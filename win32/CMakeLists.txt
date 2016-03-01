#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2015 David Demelier <markand@malikania.fr>
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
# Copy MinGW runtimes into the WITH_BINDIR directory and then put them in the Qt IFW installer hierarchy so that
# they are also installed.
#
if (IRCCD_64BITS)
	set(ARCH "amd64")
	set(LIBGCC libgcc_s_seh-1.dll)
else ()
	set(ARCH "x86")
	set(LIBGCC libgcc_s_dw2-1.dll)
endif ()

set(
	FILES
	${CMAKE_CURRENT_SOURCE_DIR}/${ARCH}/libeay32.dll
	${CMAKE_CURRENT_SOURCE_DIR}/${ARCH}/libstdc++-6.dll
	${CMAKE_CURRENT_SOURCE_DIR}/${ARCH}/libwinpthread-1.dll
	${CMAKE_CURRENT_SOURCE_DIR}/${ARCH}/ssleay32.dll
	${CMAKE_CURRENT_SOURCE_DIR}/${ARCH}/${LIBGCC}
)

install(FILES ${FILES} DESTINATION ${WITH_BINDIR})

if (IRCCD_PACKAGE)
	file(COPY ${FILES} DESTINATION ${CMAKE_BINARY_DIR}/installer/packages/base/data/${WITH_BINDIR})
endif ()

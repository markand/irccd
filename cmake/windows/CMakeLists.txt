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

#
# This file consists of installing required .dll files for Windows.
#
# It currently only supports MinGW.
#

project(windows)

#
# MinGW libraries.
# -------------------------------------------------------------------
#

if (MINGW)
	if (IRCCD_64BITS)
		set(LIBGCC_DLL libgcc_s_seh-1.dll)
	else ()
		set(LIBGCC_DLL libgcc_s_sjlj-1.dll)
	endif ()

	set(LIBWINPTHREAD_DLL libwinpthread-1.dll)
	set(LIBSTDCPP_DLL libstdc++-6.dll)
	list(APPEND DLLS LIBGCC_DLL LIBWINPTHREAD_DLL LIBSTDCPP_DLL)

	if (IRCCD_HAVE_SSL)
		set(CRYPTO_DLL libcrypto.dll)
		set(SSL_DLL libssl.dll)
		list(APPEND DLLS CRYPTO_DLL SSL_DLL)
	endif ()
endif ()

#
# Find DLL to copy/install by iterating DLLS value.
#
# Creates a cache IRCCD_WITH_<NAME> variable with the path to the DLL if found,
# otherwise, set to not found.
#
# If found, IRCCD_WITH_<NAME> is appended to IRCCD_DLLS global variable and set
# as parent scope.
#
# If not found, IRCCD_WITH_<NAME> is appended to IRCCD_DLLS_NOT_FOUND variable.
#

foreach (name ${DLLS})
	find_program(
		IRCCD_WITH_${name}
		NAMES ${${name}}
		DOC "Path to ${name} DLL"
	)

	if (EXISTS ${IRCCD_WITH_${name}})
		file(
			COPY ${IRCCD_WITH_${name}}
			DESTINATION ${CMAKE_BINARY_DIR}/bin
		)
		install(
			PROGRAMS ${IRCCD_WITH_${name}}
			COMPONENT libirccd
			DESTINATION bin
		)
		list(APPEND FOUND IRCCD_WITH_${name})
	else ()
		setg(IRCCD_PACKAGE Off)
		setg(IRCCD_PACKAGE_MSG "No (some .dll were not found)")
		list(APPEND NOT_FOUND IRCCD_WITH_${name})
	endif ()
endforeach ()

setg(IRCCD_DLLS "${FOUND}")
setg(IRCCD_DLLS_NOT_FOUND "${NOT_FOUND}")

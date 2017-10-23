#
# CMakeLists.txt -- CMake build system for irccd
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
# This file consists of installing required .dll files for Windows.
#
# It currently only supports MinGW.
#

project(win32)

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
endif ()

#
# SSL libraries.
# -------------------------------------------------------------------
#

if (HAVE_SSL)
    set(SSL_EAY_DLL libeay32.dll)
    set(SSL_LIB_DLL ssleay32.dll)
    list(APPEND DLLS SSL_EAY_DLL SSL_LIB_DLL)
endif ()

#
# Find DLL to copy/install by iterating DLLS value.
#
# Creates a cache WITH_<NAME> variable with the path to the DLL if found,
# otherwise, set to not found.
#
# If found, WITH_<NAME> is appended to IRCCD_DLLS global variable and set as
# parent scope.
#
# If not found, WITH_<NAME> is appended to IRCCD_DLLS_NOT_FOUND variable.
#

foreach (name ${DLLS})
    find_program(
        WITH_${name}
        NAMES ${${name}}
        DOC "Path to DLL"
    )

    if (EXISTS ${WITH_${name}})
        file(
            COPY ${WITH_${name}}
            DESTINATION ${IRCCD_FAKEROOTDIR}/${WITH_BINDIR}
        )
        install(
            PROGRAMS ${WITH_${name}}
            COMPONENT libirccd
            DESTINATION bin
        )
        list(APPEND IRCCD_DLLS WITH_${name})
    else ()
        set(IRCCD_PACKAGE Off PARENT_SCOPE)
        set(IRCCD_PACKAGE_MSG "No (some .dll were not found)" PARENT_SCOPE)
        list(APPEND IRCCD_DLLS_NOT_FOUND WITH_${name})
    endif ()
endforeach ()

set(IRCCD_DLLS ${IRCCD_DLLS} PARENT_SCOPE)
set(IRCCD_DLLS_NOT_FOUND ${IRCCD_DLLS_NOT_FOUND} PARENT_SCOPE)

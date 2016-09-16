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
        find_program(LIBGCC_DLL libgcc_s_seh-1.dll)
    else ()
        find_program(LIBGCC_DLL libgcc_s_sjlj-1.dll)
    endif ()

    find_program(LIBWINPTHREAD_DLL libwinpthread-1.dll)
    find_program(LIBSTDCPP_DLL libstdc++-6.dll)

    set(WITH_LIBGCC_DLL "${LIBGCC_DLL}" CACHE STRING
        "Path to libgcc dll")
    set(WITH_LIBWINPTHREAD_DLL "${LIBWINPTHREAD_DLL}" CACHE STRING
        "Path to libwinpthread dll")
    set(WITH_LIBSTDCPP_DLL "${LIBSTDCPP_DLL}" CACHE STRING
        "Path to libstdc++ dll")

    if (EXISTS ${WITH_LIBGCC_DLL} AND
        EXISTS ${WITH_LIBWINPTHREAD_DLL} AND
        EXISTS ${WITH_LIBSTDCPP_DLL})
        install(
            PROGRAMS
                ${WITH_LIBGCC_DLL}
                ${WITH_LIBWINPTHREAD_DLL}
                ${WITH_LIBSTDCPP_DLL}
            COMPONENT libirccd
            DESTINATION bin
        )
    endif ()
endif ()

#
# SSL libraries.
# -------------------------------------------------------------------
#

if (WITH_SSL)
    find_program(SSL_EAY_DLL libeay32.dll)
    find_program(SSL_LIB_DLL ssleay32.dll)

    set(WITH_SSL_EAY_DLL "${SSL_EAY_DLL}" CACHE STRING
        "Path to ssleay32.dll")
    set(WITH_SSL_LIB_DLL "${SSL_LIB_DLL}" CACHE STRING
        "Path to libeay32.dll")

    if (EXISTS ${WITH_SSL_EAY_DLL} AND EXISTS ${WITH_SSL_LIB_DLL})
        install(
            PROGRAMS
                ${WITH_SSL_EAY_DLL}
                ${WITH_SSL_LIB_DLL}
            COMPONENT libirccd
            DESTINATION bin
        )
    endif ()
endif ()

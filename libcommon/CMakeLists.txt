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

project(libcommon)

find_package(Boost 1.60 REQUIRED QUIET COMPONENTS filesystem system)

set(
    HEADERS
    ${libcommon_SOURCE_DIR}/irccd/config.hpp
    ${libcommon_SOURCE_DIR}/irccd/fs_util.hpp
    ${libcommon_SOURCE_DIR}/irccd/ini.hpp
    ${libcommon_SOURCE_DIR}/irccd/json_util.hpp
    ${libcommon_SOURCE_DIR}/irccd/network_stream.hpp
    ${libcommon_SOURCE_DIR}/irccd/options.hpp
    ${libcommon_SOURCE_DIR}/irccd/string_util.hpp
    ${libcommon_SOURCE_DIR}/irccd/system.hpp
    ${libcommon_SOURCE_DIR}/irccd/xdg.hpp
)

set(
    SOURCES
    ${libcommon_SOURCE_DIR}/irccd/config.cpp
    ${libcommon_SOURCE_DIR}/irccd/ini.cpp
    ${libcommon_SOURCE_DIR}/irccd/json_util.cpp
    ${libcommon_SOURCE_DIR}/irccd/options.cpp
    ${libcommon_SOURCE_DIR}/irccd/string_util.cpp
    ${libcommon_SOURCE_DIR}/irccd/system.cpp
)

if (NOT HAVE_SSL)
    list(APPEND FLAGS NET_NO_SSL)
endif ()

irccd_define_library(
    TARGET libcommon
    SOURCES
        ${libcommon_SOURCE_DIR}/CMakeLists.txt
        ${HEADERS}
        ${SOURCES}
    FLAGS ${FLAGS}
    LIBRARIES
        extern-json
        ${Boost_LIBRARIES}
        $<$<BOOL:${WIN32}>:shlwapi>
        $<$<BOOL:${WIN32}>:ws2_32>
        $<$<BOOL:${HAVE_SSL}>:OpenSSL::SSL>
        $<$<BOOL:${HAVE_SSL}>:OpenSSL::Crypto>
    PUBLIC_INCLUDES
        ${Boost_INCLUDE_DIRS}
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
        $<BUILD_INTERFACE:${libcommon_SOURCE_DIR}>
)

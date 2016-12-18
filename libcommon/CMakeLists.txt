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

project(libcommon)

set(
    HEADERS
    ${libcommon_SOURCE_DIR}/irccd/elapsed-timer.hpp
    ${libcommon_SOURCE_DIR}/irccd/fs.hpp
    ${libcommon_SOURCE_DIR}/irccd/ini.hpp
    ${libcommon_SOURCE_DIR}/irccd/logger.hpp
    ${libcommon_SOURCE_DIR}/irccd/net.hpp
    ${libcommon_SOURCE_DIR}/irccd/options.hpp
    ${libcommon_SOURCE_DIR}/irccd/path.hpp
    ${libcommon_SOURCE_DIR}/irccd/signals.hpp
    ${libcommon_SOURCE_DIR}/irccd/system.hpp
    ${libcommon_SOURCE_DIR}/irccd/util.hpp
    ${libcommon_SOURCE_DIR}/irccd/xdg.hpp
)

set(
    SOURCES
    ${libcommon_SOURCE_DIR}/irccd/elapsed-timer.cpp
    ${libcommon_SOURCE_DIR}/irccd/fs.cpp
    ${libcommon_SOURCE_DIR}/irccd/ini.cpp
    ${libcommon_SOURCE_DIR}/irccd/logger.cpp
    ${libcommon_SOURCE_DIR}/irccd/options.cpp
    ${libcommon_SOURCE_DIR}/irccd/path.cpp
    ${libcommon_SOURCE_DIR}/irccd/system.cpp
    ${libcommon_SOURCE_DIR}/irccd/util.cpp
)

irccd_define_library(
    TARGET libcommon
    SOURCES
        ${libcommon_SOURCE_DIR}/CMakeLists.txt
        ${HEADERS}
        ${SOURCES}
    LIBRARIES
        extern-fmt
        extern-json
        $<$<BOOL:${WITH_SSL}>:OpenSSL::SSL>
        $<$<BOOL:${WITH_SSL}>:OpenSSL::Crypto>
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${IRCCD_FAKEROOTDIR}/include/irccd>
        $<BUILD_INTERFACE:${IRCCD_FAKEROOTDIR}/include>
        $<BUILD_INTERFACE:${libcommon_SOURCE_DIR}/irccd>
        $<BUILD_INTERFACE:${libcommon_SOURCE_DIR}>
)

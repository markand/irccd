#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

project(libirccd)

find_package(Boost REQUIRED QUIET COMPONENTS timer)

set(
    HEADERS
    ${libirccd_SOURCE_DIR}/irccd/command.hpp
    ${libirccd_SOURCE_DIR}/irccd/config.hpp
    ${libirccd_SOURCE_DIR}/irccd/dynlib_plugin.hpp
    ${libirccd_SOURCE_DIR}/irccd/irccd.hpp
    ${libirccd_SOURCE_DIR}/irccd/plugin.hpp
    ${libirccd_SOURCE_DIR}/irccd/rule.hpp
    ${libirccd_SOURCE_DIR}/irccd/server.hpp
    ${libirccd_SOURCE_DIR}/irccd/service.hpp
    ${libirccd_SOURCE_DIR}/irccd/transport.hpp
)

set(
    SOURCES
    ${libirccd_SOURCE_DIR}/irccd/command.cpp
    ${libirccd_SOURCE_DIR}/irccd/config.cpp
    ${libirccd_SOURCE_DIR}/irccd/dynlib_plugin.cpp
    ${libirccd_SOURCE_DIR}/irccd/irccd.cpp
    ${libirccd_SOURCE_DIR}/irccd/plugin.cpp
    ${libirccd_SOURCE_DIR}/irccd/rule.cpp
    ${libirccd_SOURCE_DIR}/irccd/server.cpp
    ${libirccd_SOURCE_DIR}/irccd/service.cpp
    ${libirccd_SOURCE_DIR}/irccd/transport.cpp
)

irccd_define_library(
    TARGET libirccd
    SOURCES
        ${libirccd_SOURCE_DIR}/CMakeLists.txt
        ${HEADERS}
        ${SOURCES}
    LIBRARIES
        Boost::timer
        $<$<BOOL:${IRCCD_SYSTEM_LINUX}>:dl>
        $<$<BOOL:${IRCCD_SYSTEM_MAC}>:resolv>
        extern-ircclient
        libcommon
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${libirccd_SOURCE_DIR}/irccd>
        $<BUILD_INTERFACE:${libirccd_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${WITH_INCLUDEDIR}>
)

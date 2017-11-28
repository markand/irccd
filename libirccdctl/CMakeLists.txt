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

project(libirccdctl)

set(
    HEADERS
    ${libirccdctl_SOURCE_DIR}/irccd/ctl/basic_connection.hpp
    ${libirccdctl_SOURCE_DIR}/irccd/ctl/controller.hpp
    ${libirccdctl_SOURCE_DIR}/irccd/ctl/connection.hpp
    ${libirccdctl_SOURCE_DIR}/irccd/ctl/ip_connection.hpp
)

set(
    SOURCES
    ${libirccdctl_SOURCE_DIR}/irccd/ctl/controller.cpp
    ${libirccdctl_SOURCE_DIR}/irccd/ctl/ip_connection.cpp
)

if (NOT IRCCD_SYSTEM_WINDOWS)
    list(APPEND LIBRARIES -pthread)
    list(APPEND HEADERS ${libirccdctl_SOURCE_DIR}/irccd/ctl/local_connection.hpp)
    list(APPEND SOURCES ${libirccdctl_SOURCE_DIR}/irccd/ctl/local_connection.cpp)
endif ()

irccd_define_library(
    TARGET libirccdctl
    SOURCES
        ${libirccdctl_SOURCE_DIR}/CMakeLists.txt
        ${HEADERS}
        ${SOURCES}
    LIBRARIES
        ${LIBRARIES}
        libcommon
        libirccd
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${libirccdctl_SOURCE_DIR}>
)

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

project(libirccd-core)

find_package(Boost 1.60 REQUIRED QUIET COMPONENTS filesystem system)

set(
    HEADERS
    ${libirccd-core_SOURCE_DIR}/irccd/acceptor.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/config.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/connector.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/fs_util.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/ini.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/ini_util.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/json_util.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/options.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/socket_acceptor.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/socket_connector.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/socket_stream.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/stream.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/string_util.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/system.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/tls_acceptor.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/tls_connector.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/tls_stream.hpp
    ${libirccd-core_SOURCE_DIR}/irccd/xdg.hpp
)

set(
    SOURCES
    ${libirccd-core_SOURCE_DIR}/irccd/config.cpp
    ${libirccd-core_SOURCE_DIR}/irccd/ini.cpp
    ${libirccd-core_SOURCE_DIR}/irccd/options.cpp
    ${libirccd-core_SOURCE_DIR}/irccd/string_util.cpp
    ${libirccd-core_SOURCE_DIR}/irccd/system.cpp
)

irccd_define_library(
    TARGET libirccd-core
    EXPORT
    HEADERS ${HEADERS}
    HEADERS_DIRECTORY irccd
    SOURCES
        ${libirccd-core_SOURCE_DIR}/CMakeLists.txt
        ${SOURCES}
    LIBRARIES
        libjson
        Threads::Threads
        Boost::filesystem
        Boost::system
        $<$<BOOL:${IRCCD_HAVE_SSL}>:OpenSSL::Crypto>
        $<$<BOOL:${IRCCD_HAVE_SSL}>:OpenSSL::SSL>
        $<$<BOOL:${APPLE}>:resolv>
        $<$<BOOL:${WIN32}>:mswsock>
        $<$<BOOL:${WIN32}>:shlwapi>
        $<$<BOOL:${WIN32}>:ws2_32>
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:dl>
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
        $<BUILD_INTERFACE:${libirccd-core_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/irccd/extern>
)

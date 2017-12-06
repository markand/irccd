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
    ${libirccd_SOURCE_DIR}/irccd/daemon/basic_transport_client.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/basic_transport_server.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/command.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/command_service.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/config.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/dynlib_plugin.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/ip_transport_server.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/irccd.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/irc.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/local_transport_server.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/plugin.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/plugin_service.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/rule.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/rule_service.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/server.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/server_service.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/transport_client.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/transport_server.hpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/transport_service.hpp
    $<$<BOOL:${HAVE_SSL}>:${libirccd_SOURCE_DIR}/irccd/daemon/tls_transport_server.hpp>
)

set(
    SOURCES
    ${libirccd_SOURCE_DIR}/irccd/daemon/command.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/command_service.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/config.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/dynlib_plugin.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/irccd.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/irc.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/plugin.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/plugin_service.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/rule.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/rule_service.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/server.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/server_service.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/transport_client.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/transport_server.cpp
    ${libirccd_SOURCE_DIR}/irccd/daemon/transport_service.cpp
    $<$<BOOL:${HAVE_SSL}>:${libirccd_SOURCE_DIR}/irccd/daemon/tls_transport_server.cpp>
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
        libcommon
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${libirccd_SOURCE_DIR}/irccd/daemon>
        $<BUILD_INTERFACE:${libirccd_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${WITH_INCLUDEDIR}>
)

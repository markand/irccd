# # CMakeLists.txt -- CMake build system for irccd
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

project(irccdctl)

set(
    SOURCES
    ${irccdctl_SOURCE_DIR}/CMakeLists.txt
    ${irccdctl_SOURCE_DIR}/cli.cpp
    ${irccdctl_SOURCE_DIR}/cli.hpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-config.cpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-config.hpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-info.cpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-info.hpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-list.cpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-list.hpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-load.cpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-load.hpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-reload.cpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-reload.hpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-unload.cpp
    ${irccdctl_SOURCE_DIR}/cli-plugin-unload.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-cmode.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-cmode.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-cnotice.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-cnotice.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-connect.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-connect.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-disconnect.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-disconnect.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-info.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-info.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-invite.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-invite.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-join.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-join.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-kick.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-kick.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-list.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-list.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-me.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-me.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-message.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-message.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-mode.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-mode.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-nick.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-nick.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-notice.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-notice.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-part.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-part.hpp
    ${irccdctl_SOURCE_DIR}/cli-server-reconnect.cpp
    ${irccdctl_SOURCE_DIR}/cli-server-reconnect.hpp
    ${irccdctl_SOURCE_DIR}/main.cpp
)

irccd_define_executable(
    TARGET irccdctl
    DESCRIPTION "Irccd controller."
    SOURCES ${SOURCES}
    LIBRARIES libirccdctl
    INCLUDES ${irccdctl_SOURCE_DIR}
)

if (UNIX)
    set_target_properties(irccdctl PROPERTIES LINK_FLAGS -pthread)
endif ()


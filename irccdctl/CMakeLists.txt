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

project(irccdctl)

set(
    SOURCES
    ${irccdctl_SOURCE_DIR}/CMakeLists.txt
    ${irccdctl_SOURCE_DIR}/alias.cpp
    ${irccdctl_SOURCE_DIR}/alias.hpp
    ${irccdctl_SOURCE_DIR}/cli.cpp
    ${irccdctl_SOURCE_DIR}/cli.cpp
    ${irccdctl_SOURCE_DIR}/cli.hpp
    ${irccdctl_SOURCE_DIR}/cli.hpp
    ${irccdctl_SOURCE_DIR}/main.cpp
    ${irccdctl_SOURCE_DIR}/plugin_config_cli.cpp
    ${irccdctl_SOURCE_DIR}/plugin_config_cli.hpp
    ${irccdctl_SOURCE_DIR}/plugin_info_cli.cpp
    ${irccdctl_SOURCE_DIR}/plugin_info_cli.hpp
    ${irccdctl_SOURCE_DIR}/plugin_list_cli.cpp
    ${irccdctl_SOURCE_DIR}/plugin_list_cli.hpp
    ${irccdctl_SOURCE_DIR}/plugin_load_cli.cpp
    ${irccdctl_SOURCE_DIR}/plugin_load_cli.hpp
    ${irccdctl_SOURCE_DIR}/plugin_reload_cli.cpp
    ${irccdctl_SOURCE_DIR}/plugin_reload_cli.hpp
    ${irccdctl_SOURCE_DIR}/plugin_unload_cli.cpp
    ${irccdctl_SOURCE_DIR}/plugin_unload_cli.hpp
    ${irccdctl_SOURCE_DIR}/rule_add_cli.cpp
    ${irccdctl_SOURCE_DIR}/rule_add_cli.hpp
    ${irccdctl_SOURCE_DIR}/rule_edit_cli.cpp
    ${irccdctl_SOURCE_DIR}/rule_edit_cli.hpp
    ${irccdctl_SOURCE_DIR}/rule_info_cli.cpp
    ${irccdctl_SOURCE_DIR}/rule_info_cli.hpp
    ${irccdctl_SOURCE_DIR}/rule_list_cli.cpp
    ${irccdctl_SOURCE_DIR}/rule_list_cli.hpp
    ${irccdctl_SOURCE_DIR}/rule_move_cli.cpp
    ${irccdctl_SOURCE_DIR}/rule_move_cli.hpp
    ${irccdctl_SOURCE_DIR}/rule_remove_cli.cpp
    ${irccdctl_SOURCE_DIR}/rule_remove_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_connect_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_connect_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_disconnect_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_disconnect_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_info_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_info_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_invite_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_invite_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_join_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_join_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_kick_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_kick_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_list_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_list_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_me_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_me_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_message_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_message_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_mode_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_mode_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_nick_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_nick_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_notice_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_notice_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_part_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_part_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_reconnect_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_reconnect_cli.hpp
    ${irccdctl_SOURCE_DIR}/server_topic_cli.cpp
    ${irccdctl_SOURCE_DIR}/server_topic_cli.hpp
    ${irccdctl_SOURCE_DIR}/watch_cli.cpp
    ${irccdctl_SOURCE_DIR}/watch_cli.hpp
)

irccd_define_executable(
    TARGET irccdctl
    DESCRIPTION "Irccd controller."
    SOURCES ${SOURCES}
    LIBRARIES libirccdctl
)

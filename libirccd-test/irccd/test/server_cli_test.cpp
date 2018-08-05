/*
 * server_cli_test.cpp -- test fixture for irccdctl frontend (server support)
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <irccd/daemon/command.hpp>
#include <irccd/daemon/transport_service.hpp>

#include "server_cli_test.hpp"

namespace irccd {

server_cli_test::server_cli_test()
    : server_(new mock_server(irccd_.get_service(), "test", "localhost"))
{
    irccd_.servers().add(server_);
    server_->clear();

    irccd_.transports().get_commands().push_back(std::make_unique<server_connect_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_disconnect_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_info_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_invite_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_join_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_kick_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_list_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_me_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_message_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_mode_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_nick_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_notice_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_part_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_reconnect_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<server_topic_command>());
}

} // !irccd

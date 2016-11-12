/*
 * main.cpp -- irccd controller main
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include "client.hpp"
#include "irccdctl.hpp"

#include "cli-plugin-reload.hpp"
#include "cli-plugin-unload.hpp"
#include "cli-server-cmode.hpp"
#include "cli-server-cnotice.hpp"
#include "cli-server-connect.hpp"
#include "cli-server-disconnect.hpp"
#include "cli-server-info.hpp"
#include "cli-server-invite.hpp"
#include "cli-server-join.hpp"
#include "cli-server-kick.hpp"
#include "cli-server-list.hpp"
#include "cli-server-me.hpp"
#include "cli-server-message.hpp"
#include "cli-server-mode.hpp"
#include "cli-server-nick.hpp"
#include "cli-server-notice.hpp"
#include "cli-server-part.hpp"
#include "cli-server-reconnect.hpp"

using namespace irccd;

int main(int, char **)
{
    try {
        Irccdctl ctl(std::make_unique<Client>());
        ctl.client().connect(net::local::create("/tmp/irccd.sock"));

        cli::ServerReconnectCli command;
        command.exec(ctl, {"local"});
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }
}

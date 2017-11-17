/*
 * transport_service.cpp -- transport service
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <cassert>

#include "command.hpp"
#include "irccd.hpp"
#include "logger.hpp"
#include "service.hpp"
#include "transport_client.hpp"
#include "transport_server.hpp"
#include "transport_service.hpp"

namespace irccd {

void transport_service::handle_command(std::shared_ptr<transport_client> tc, const nlohmann::json& object)
{
    assert(object.is_object());

    irccd_.post([=] (irccd&) {
        auto name = object.find("command");
        if (name == object.end() || !name->is_string()) {
            // TODO: send error.
            log::warning("invalid command object");
            return;
        }

        auto cmd = irccd_.commands().find(*name);

        if (!cmd)
            tc->error(*name, "command does not exist");
        else {
            try {
                cmd->exec(irccd_, *tc, object);
            } catch (const std::exception& ex) {
                tc->error(cmd->name(), ex.what());
            }
        }
    });
}

void transport_service::do_accept(std::shared_ptr<transport_server> ts)
{
    ts->accept([this, ts] (auto client, auto code) {
        if (code) {
            log::warning() << "transport: " << code.message() << std::endl;
        } else {
            client->recv([this, client] (auto json, auto code) {
                if (code)
                    log::warning() << "transport: " << code.message() << std::endl;
                else
                    handle_command(client, json);
            });
        }

        do_accept(ts);
    });
}

transport_service::transport_service(irccd& irccd) noexcept
    : irccd_(irccd)
{
}

void transport_service::add(std::shared_ptr<transport_server> ts)
{
    assert(ts);

    do_accept(ts);
    servers_.push_back(std::move(ts));
}

void transport_service::broadcast(const nlohmann::json& json)
{
    assert(json.is_object());

    for (const auto& servers : servers_)
        for (const auto& client : servers->clients())
            client->send(json);
}

} // !irccd

/*
 * transport_service.cpp -- transport service
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

#include <irccd/sysconfig.hpp>

#include <cassert>

#include <irccd/daemon/command.hpp>
#include <irccd/daemon/ip_transport_server.hpp>
#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/transport_util.hpp>
#include <irccd/daemon/transport_client.hpp>

#include "transport_service.hpp"

namespace irccd {

namespace {

} // !namespace

void transport_service::handle_command(std::shared_ptr<transport_client> tc, const nlohmann::json& object)
{
    assert(object.is_object());

    auto name = object.find("command");

    if (name == object.end() || !name->is_string()) {
        tc->error(irccd_error::invalid_message);
        return;
    }

    auto cmd = std::find_if(commands_.begin(), commands_.end(), [&] (const auto& cptr) {
        return cptr->get_name() == name->template get<std::string>();
    });

    if (cmd == commands_.end())
        tc->error(irccd_error::invalid_command, name->get<std::string>());
    else {
        try {
            (*cmd)->exec(irccd_, *tc, object);
        } catch (const boost::system::system_error& ex) {
            tc->error(ex.code(), (*cmd)->get_name());
        } catch (const std::exception& ex) {
            irccd_.log().warning() << "transport: unknown error not reported" << std::endl;
            irccd_.log().warning() << "transport: " << ex.what() << std::endl;
        }
    }
}

void transport_service::do_recv(std::shared_ptr<transport_client> tc)
{
    tc->recv([this, tc] (auto code, auto json) {
        switch (code.value()) {
        case boost::system::errc::network_down:
            irccd_.log().warning("transport: client disconnected");
            break;
        case boost::system::errc::invalid_argument:
            tc->error(irccd_error::invalid_message);
            break;
        default:
            handle_command(tc, json);

            if (tc->state() == transport_client::state_t::ready)
                do_recv(std::move(tc));

            break;
        }
    });
}

void transport_service::do_accept(transport_server& ts)
{
    ts.accept([this, &ts] (auto code, auto client) {
        if (code)
            irccd_.log().warning() << "transport: new client error: " << code.message() << std::endl;
        else {
            do_accept(ts);
            do_recv(std::move(client));

            irccd_.log().info() << "transport: new client connected" << std::endl;
        }
    });
}

transport_service::transport_service(irccd& irccd) noexcept
    : irccd_(irccd)
{
}

transport_service::~transport_service() noexcept = default;

void transport_service::add(std::unique_ptr<transport_server> ts)
{
    assert(ts);

    do_accept(*ts);
    servers_.push_back(std::move(ts));
}

void transport_service::broadcast(const nlohmann::json& json)
{
    assert(json.is_object());

    for (const auto& servers : servers_)
        for (const auto& client : servers->clients())
            client->send(json);
}

void transport_service::load(const config& cfg) noexcept
{
    for (const auto& section : cfg.doc()) {
        if (section.key() != "transport")
            continue;

        try {
            add(transport_util::from_config(irccd_.service(), section));
        } catch (const std::exception& ex) {
            irccd_.log().warning() << "transport: " << ex.what() << std::endl;
        }
    }
}

} // !irccd

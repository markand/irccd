/*
 * cli.cpp -- command line for irccdctl
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

#include <json.hpp>

#include "cli.hpp"
#include "irccdctl.hpp"
#include "util.hpp"
#include "elapsed-timer.hpp"

namespace irccd {

void Cli::check(const nlohmann::json &response)
{
    if (!util::json::getBool(response, "status", false)) {
        auto error = util::json::getString(response, "error");

        if (error.empty())
            throw std::runtime_error("command failed with an unknown error");

        throw std::runtime_error(error);
    }
}

nlohmann::json Cli::request(Irccdctl &irccdctl, nlohmann::json args)
{
    auto msg = nlohmann::json();

    if (!args.is_object())
        args = nlohmann::json::object();

    args.push_back({"command", m_name});
    irccdctl.client().request(args);

    auto id = irccdctl.client().onMessage.connect([&] (auto input) {
        msg = std::move(input);
    });

    try {
        ElapsedTimer timer;

        while (!msg.is_object() && timer.elapsed() < 3000)
            util::poller::poll(3000 - timer.elapsed(), irccdctl);
    } catch (const std::exception &) {
        irccdctl.client().onMessage.disconnect(id);
        throw;
    }

    irccdctl.client().onMessage.disconnect(id);

    std::cout << msg << std::endl;

    if (!msg.is_object())
        throw std::runtime_error("no response received");
    if (util::json::getString(msg, "command") != m_name)
        throw std::runtime_error("unexpected command result received");

    check(msg);

    return msg;
}

void Cli::call(Irccdctl &irccdctl, nlohmann::json args)
{
    check(request(irccdctl, args));
}

} // !irccd

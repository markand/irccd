/*
 * controller.cpp -- main irccdctl interface
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

#include <irccd/network_errc.hpp>
#include <irccd/sysconfig.hpp>
#include <irccd/json_util.hpp>

#include "controller.hpp"
#include "connection.hpp"

namespace irccd {

namespace ctl {

void controller::flush_recv()
{
    if (rqueue_.empty())
        return;

    conn_.recv([this] (auto code, auto json) {
        rqueue_.front()(code, std::move(json));
        rqueue_.pop_front();

        if (!code)
            flush_recv();
    });
}

void controller::flush_send()
{
    if (squeue_.empty())
        return;

    conn_.send(squeue_.front().first, [this] (auto code) {
        if (squeue_.front().second)
            squeue_.front().second(code, squeue_.front().first);

        squeue_.pop_front();

        if (!code)
            flush_send();
    });
}

void controller::authenticate(connect_t handler, nlohmann::json info)
{
    auto cmd = nlohmann::json::object({
        { "command", "authenticate" },
        { "password", password_ }
    });

    send(std::move(cmd), [handler, info, this] (auto code, auto) {
        if (code) {
            handler(std::move(code), nullptr);
            return;
        }

        recv([handler, info, this] (auto code, auto message) {
            if (message["error"].is_string())
                code = network_errc::invalid_auth;

            handler(std::move(code), std::move(info));
        });
    });
}

void controller::verify(connect_t handler)
{
    recv([handler, this] (auto code, auto message) {
        if (code) {
            handler(std::move(code), std::move(message));
            return;
        }

        if (json_util::to_string(message["program"]) != "irccd")
            handler(network_errc::invalid_program, std::move(message));
        else if (json_util::to_int(message["major"]) != IRCCD_VERSION_MAJOR)
            handler(network_errc::invalid_version, std::move(message));
        else {
            if (!password_.empty())
                authenticate(std::move(handler), message);
            else
                handler(code, std::move(message));
        }
    });
}

void controller::connect(connect_t handler)
{
    assert(handler);

    conn_.connect([handler, this] (auto code) {
        if (code)
            handler(std::move(code), nullptr);
        else
            verify(std::move(handler));
    });
}

void controller::recv(recv_t handler)
{
    assert(handler);

    auto in_progress = !rqueue_.empty();

    rqueue_.push_back(std::move(handler));

    if (!in_progress)
        flush_recv();
}

void controller::send(nlohmann::json message, send_t handler)
{
    assert(message.is_object());

    auto in_progress = !squeue_.empty();

    squeue_.emplace_back(std::move(message), std::move(handler));

    if (!in_progress)
        flush_send();
}

} // !ctl

} // !irccd

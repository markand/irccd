/*
 * controller.cpp -- main irccdctl interface
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

#include <cassert>

#include <irccd/sysconfig.hpp>
#include <irccd/json_util.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server.hpp>
#include <irccd/daemon/plugin.hpp>
#include <irccd/daemon/rule.hpp>

#include "controller.hpp"
#include "connection.hpp"

namespace irccd {

namespace ctl {

void controller::authenticate(connect_t handler, nlohmann::json info)
{
    auto cmd = nlohmann::json::object({
        { "command", "auth" },
        { "password", password_ }
    });

    send(std::move(cmd), [handler, info, this] (auto code) {
        if (code) {
            handler(std::move(code), nullptr);
            return;
        }

        recv([handler, info] (auto code, auto) {
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

        const json_util::parser parser(message);
        const auto program = parser.get<std::string>("program");
        const auto major = parser.get<int>("major");

        if (!program && *program != "irccd")
            handler(irccd_error::not_irccd, std::move(message));
        else if (major && *major != IRCCD_VERSION_MAJOR)
            handler(irccd_error::incompatible_version, std::move(message));
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

void controller::recv(network_recv_handler handler)
{
    assert(handler);

    // TODO: ensure connected.
    conn_.recv([handler] (auto code, auto msg) {
        if (code) {
            handler(std::move(code), std::move(msg));
            return;
        }

        const json_util::parser parser(msg);
        const auto e = parser.get<int>("error");
        const auto c = parser.get<std::string>("errorCategory");

        if (e && c) {
            if (*c == "irccd")
                code = make_error_code(static_cast<irccd_error::error>(*e));
            else if (*c == "server")
                code = make_error_code(static_cast<server_error::error>(*e));
            else if (*c == "plugin")
                code = make_error_code(static_cast<plugin_error::error>(*e));
            else if (*c == "rule")
                code = make_error_code(static_cast<rule_error::error>(*e));
        }

        handler(std::move(code), std::move(msg));
    });
}

void controller::send(nlohmann::json message, network_send_handler handler)
{
    assert(message.is_object());

    // TODO: ensure connected.
    conn_.send(std::move(message), std::move(handler));
}

} // !ctl

} // !irccd

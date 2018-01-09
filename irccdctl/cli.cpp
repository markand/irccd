/*
 * cli.cpp -- command line for irccdctl
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

#include <boost/system/system_error.hpp>

#include <irccd/json_util.hpp>
#include <irccd/options.hpp>
#include <irccd/string_util.hpp>

#include <irccd/ctl/controller.hpp>

#include "cli.hpp"

namespace irccd {

namespace ctl {

void cli::recv_response(ctl::controller& ctl, nlohmann::json req, handler_t handler)
{
    ctl.recv([&ctl, req, handler, this] (auto code, auto message) {
        if (code)
            throw boost::system::system_error(code);

        auto c = json_util::to_string(message["command"]);

        if (c != req["command"].get<std::string>()) {
            recv_response(ctl, std::move(req), std::move(handler));
            return;
        }

        if (handler)
            handler(std::move(message));
    });
}

void cli::request(ctl::controller& ctl, nlohmann::json req, handler_t handler)
{
    ctl.send(req, [&ctl, req, handler, this] (auto code) {
        if (code)
            throw boost::system::system_error(code);

        recv_response(ctl, std::move(req), std::move(handler));
    });
}

} // !cli

} // !irccd

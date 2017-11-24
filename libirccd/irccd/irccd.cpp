/*
 * irccd.cpp -- main irccd class
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

#include "command_service.hpp"
#include "irccd.hpp"
#include "plugin_service.hpp"
#include "rule_service.hpp"
#include "server_service.hpp"
#include "transport_service.hpp"

namespace irccd {

irccd::irccd(boost::asio::io_service& service, std::string config)
    : config_(std::move(config))
    , service_(service)
    , command_service_(std::make_unique<command_service>())
    , server_service_(std::make_unique<server_service>(*this))
    , tpt_service_(std::make_unique<transport_service>(*this))
    , rule_service_(std::make_unique<rule_service>())
    , plugin_service_(std::make_unique<plugin_service>(*this))
{
}

irccd::~irccd() = default;

} // !irccd

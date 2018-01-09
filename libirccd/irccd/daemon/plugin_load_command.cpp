/*
 * plugin_load_command.cpp -- implementation of plugin-load transport command
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

#include <irccd/json_util.hpp>

#include "irccd.hpp"
#include "plugin_load_command.hpp"
#include "plugin_service.hpp"
#include "transport_client.hpp"

namespace irccd {

std::string plugin_load_command::get_name() const noexcept
{
    return "plugin-load";
}

void plugin_load_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    irccd.plugins().load(json_util::require_identifier(args, "plugin"));
    client.success("plugin-load");
}

} // !irccd

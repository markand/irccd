/*
 * plugin_cli_test.cpp -- test fixture for irccdctl frontend (plugins support)
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

#include "plugin_cli_test.hpp"

namespace irccd {

plugin_cli_test::plugin_cli_test()
{
    irccd_.transports().get_commands().push_back(std::make_unique<plugin_config_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<plugin_info_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<plugin_list_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<plugin_load_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<plugin_reload_command>());
    irccd_.transports().get_commands().push_back(std::make_unique<plugin_unload_command>());
}

} // !irccd

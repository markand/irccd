/*
 * cmd-help.cpp -- implementation of irccdctl help
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

#include "cmd-help.hpp"
#include "irccdctl.hpp"
#include "logger.hpp"

namespace irccd {

namespace command {

HelpCommand::HelpCommand()
    : Command("help", "General", "Get help about a command")
{
}

std::vector<Command::Arg> HelpCommand::args() const
{
    return {{ "command", true }};
}

nlohmann::json HelpCommand::request(Irccdctl &irccdctl, const CommandRequest &args) const
{
    auto it = irccdctl.commandService().find(args.arg(0U));

    if (!it)
        log::warning() << "there is no command named: " << args.arg(0U) << std::endl;
    else
        log::warning() << it->help() << std::flush;

    return nullptr;
}

} // !command

} // !irccd

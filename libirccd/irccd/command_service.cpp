/*
 * command_service.cpp -- command service
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

namespace irccd {

bool command_service::contains(const std::string& name) const noexcept
{
    return find(name) != nullptr;
}

std::shared_ptr<command> command_service::find(const std::string& name) const noexcept
{
    auto it = std::find_if(commands_.begin(), commands_.end(), [&] (const auto& cmd) {
        return cmd->name() == name;
    });

    return it == commands_.end() ? nullptr : *it;
}

void command_service::add(std::shared_ptr<command> command)
{
    auto it = std::find_if(commands_.begin(), commands_.end(), [&] (const auto& cmd) {
        return cmd->name() == command->name();
    });

    if (it != commands_.end())
        *it = std::move(command);
    else
        commands_.push_back(std::move(command));
}

} // !irccd

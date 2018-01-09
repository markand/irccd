/*
 * command_service.hpp -- command service
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

#ifndef IRCCD_DAEMON_COMMAND_SERVICE_HPP
#define IRCCD_DAEMON_COMMAND_SERVICE_HPP

/**
 * \file command_service.hpp
 * \brief Command service.
 */

#include <memory>
#include <vector>

#include "command.hpp"

namespace irccd {

/**
 * \brief Store remote commands.
 * \ingroup services
 */
class command_service {
private:
    std::vector<std::shared_ptr<command>> commands_;

public:
    /**
     * Get all commands.
     *
     * \return the list of commands.
     */
    inline const std::vector<std::shared_ptr<command>>& commands() const noexcept
    {
        return commands_;
    }

    /**
     * Tells if a command exists.
     *
     * \param name the command name
     * \return true if the command exists
     */
    bool contains(const std::string& name) const noexcept;

    /**
     * Find a command by name.
     *
     * \param name the command name
     * \return the command or empty one if not found
     */
    std::shared_ptr<command> find(const std::string& name) const noexcept;

    /**
     * Add a command or replace existing one.
     *
     * \pre command != nullptr
     * \param command the command name
     */
    void add(std::shared_ptr<command> command);
};

} // !irccd

#endif // !IRCCD_DAEMON_COMMAND_SERVICE_HPP

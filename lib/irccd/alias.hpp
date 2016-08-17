/*
 * alias.hpp -- create irccdctl aliases
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

#ifndef IRCCD_ALIAS_HPP
#define IRCCD_ALIAS_HPP

/**
 * \file alias.hpp
 * \brief Create irccdctl aliases.
 */

#include <ostream>
#include <string>
#include <vector>

#include "sysconfig.hpp"

namespace irccd {

/**
 * \class AliasArg
 * \brief Describe an alias argument.
 *
 * When the user specify arguments, it can precise an applied argument or a
 * placeholder that will be substituted during command line invocation.
 *
 * Placeholders are placed using %n where n is an integer starting from 0.
 */
class AliasArg {
private:
    std::string m_value;
    bool m_isPlaceholder;

public:
    /**
     * Construct an argument.
     *
     * \pre value must not be empty
     * \param value the value
     */
    IRCCD_EXPORT AliasArg(std::string value);

    /**
     * Check if the argument is a placeholder.
     *
     * \return true if the argument is a placeholder
     */
    inline bool isPlaceholder() const noexcept
    {
        return m_isPlaceholder;
    }

    /**
     * Get the placeholder index (e.g %0 returns 0)
     *
     * \pre isPlaceholder() must return true
     * \return the position
     */
    IRCCD_EXPORT unsigned index() const noexcept;

    /**
     * Get the real value.
     *
     * \pre isPlaceholder() must return false
     * \return the value
     */
    IRCCD_EXPORT const std::string &value() const noexcept;

    /**
     * Output the alias to the stream.
     *
     * \param out the output stream
     * \return out
     */
    IRCCD_EXPORT friend std::ostream &operator<<(std::ostream &out, const AliasArg &);
};

/**
 * \class AliasCommand
 * \brief Describe a user-provided alias command.
 *
 * An alias command is just a command with a set of applied or placeholders
 * arguments.
 */
class AliasCommand {
private:
    std::string m_command;
    std::vector<AliasArg> m_args;

public:
    /**
     * Create an alias command.
     *
     * \param command the command
     * \param args the arguments
     */
    inline AliasCommand(std::string command, std::vector<AliasArg> args = {}) noexcept
        : m_command(std::move(command))
        , m_args(std::move(args))
    {
    }

    /**
     * Get the command to execute.
     *
     * \return the command name
     */
    inline const std::string &command() const noexcept
    {
        return m_command;
    }

    /**
     * Get the arguments.
     *
     * \return the arguments
     */
    inline const std::vector<AliasArg> &args() const noexcept
    {
        return m_args;
    }
};

/**
 * \class Alias
 * \brief A set of commands to execute with their arguments.
 *
 * An alias is a composition of AliasCommand, typically, the user is able to set
 * an alias that execute a list of specified commands in order they are defined.
 */
class Alias : public std::vector<AliasCommand> {
private:
    std::string m_name;

public:
    /**
     * Create an alias.
     *
     * \param name the alias name
     */
    inline Alias(std::string name) noexcept
        : m_name(std::move(name))
    {
    }

    /**
     * Get the alias name.
     *
     * \return the name
     */
    inline const std::string &name() const noexcept
    {
        return m_name;
    }
};

} // !irccd

#endif // !IRCCD_ALIAS_HPP

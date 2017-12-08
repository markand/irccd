/*
 * alias.hpp -- create irccdctl aliases
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

#ifndef IRCCD_CTL_ALIAS_HPP
#define IRCCD_CTL_ALIAS_HPP

/**
 * \file alias.hpp
 * \brief Create irccdctl aliases.
 */

#include <irccd/sysconfig.hpp>

#include <ostream>
#include <string>
#include <vector>

namespace irccd {

namespace ctl {

/**
 * \brief Describe an alias argument.
 *
 * When the user specify arguments, it can precise an applied argument or a
 * placeholder that will be substituted during command line invocation.
 *
 * Placeholders are placed using %n where n is an integer starting from 0.
 */
class alias_arg {
private:
    std::string value_;
    bool is_placeholder_;

public:
    /**
     * Construct an argument.
     *
     * \pre value must not be empty
     * \param value the value
     */
    alias_arg(std::string value);

    /**
     * Check if the argument is a placeholder.
     *
     * \return true if the argument is a placeholder
     */
    inline bool is_placeholder() const noexcept
    {
        return is_placeholder_;
    }

    /**
     * Get the placeholder index (e.g %0 returns 0)
     *
     * \pre is_placeholder() must return true
     * \return the position
     */
    unsigned index() const noexcept;

    /**
     * Get the real value.
     *
     * \pre is_placeholder() must return false
     * \return the value
     */
    const std::string& value() const noexcept;

    /**
     * Output the alias to the stream.
     *
     * \param out the output stream
     * \return out
     */
    friend std::ostream& operator<<(std::ostream& out, const alias_arg&);
};

/**
 * \brief Describe a user-provided alias command.
 *
 * An alias command is just a command with a set of applied or placeholders
 * arguments.
 */
class alias_command {
private:
    std::string command_;
    std::vector<alias_arg> args_;

public:
    /**
     * Create an alias command.
     *
     * \param command the command
     * \param args the arguments
     */
    inline alias_command(std::string command, std::vector<alias_arg> args = {}) noexcept
        : command_(std::move(command))
        , args_(std::move(args))
    {
    }

    /**
     * Get the command to execute.
     *
     * \return the command name
     */
    inline const std::string& command() const noexcept
    {
        return command_;
    }

    /**
     * Get the arguments.
     *
     * \return the arguments
     */
    inline const std::vector<alias_arg>& args() const noexcept
    {
        return args_;
    }
};

/**
 * \brief A set of commands to execute with their arguments.
 *
 * An alias is a composition of alias_command, typically, the user is able to
 * set an alias that execute a list of specified commands in order they are
 * defined.
 */
class alias : public std::vector<alias_command> {
private:
    std::string name_;

public:
    /**
     * Create an alias.
     *
     * \param name the alias name
     */
    inline alias(std::string name) noexcept
        : name_(std::move(name))
    {
    }

    /**
     * Get the alias name.
     *
     * \return the name
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }
};

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_ALIAS_HPP

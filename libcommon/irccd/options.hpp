/*
 * options.h -- parse Unix command line options
 *
 * Copyright (c) 2015 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_OPTIONS_HPP
#define IRCCD_OPTIONS_HPP

/**
 * \file options.hpp
 * \brief Basic Unix options parser.
 */

#include <exception>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "sysconfig.hpp"

namespace irccd {

/**
 * Namespace for options parsing.
 */
namespace option {

/**
 * \brief This exception is thrown when an invalid option has been found.
 */
class InvalidOption : public std::exception {
private:
    std::string message;

public:
    /**
     * The invalid option given.
     */
    std::string argument;

    /**
     * Construct the exception.
     *
     * \param arg the argument missing
     */
    inline InvalidOption(std::string arg)
        : argument(std::move(arg))
    {
        message = std::string("invalid option: ") + argument;
    }

    /**
     * Get the error message.
     *
     * \return the error message
     */
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};

/**
 * \brief This exception is thrown when an option requires a value and no value has been given.
 */
class MissingValue : public std::exception {
private:
    std::string m_message;
    std::string m_option;

public:
    /**
     * Construct the exception.
     *
     * \param option the option that requires a value
     */
    inline MissingValue(std::string option)
        : m_option(std::move(option))
    {
        m_message = std::string("missing argument for: ") + m_option;
    }

    /**
     * Get the options that requires a value.
     *
     * \return the option name
     */
    inline const std::string &option() const noexcept
    {
        return m_option;
    }

    /**
     * Get the error message.
     *
     * \return the error message
     */
    const char *what() const noexcept override
    {
        return m_message.c_str();
    }
};

/**
 * Packed multimap of options.
 */
using Result = std::multimap<std::string, std::string>;

/**
 * Define the allowed options.
 */
using Options = std::map<std::string, bool>;

/**
 * Extract the command line options and return a result.
 *
 * \param args the arguments
 * \param definition
 * \warning the arguments vector is modified in place to remove parsed options
 * \throw MissingValue
 * \throw InvalidOption
 */
IRCCD_EXPORT Result read(std::vector<std::string> &args, const Options &definition);

/**
 * Overloaded function for usage with main() arguments.
 *
 * \param argc the number of arguments
 * \param argv the argument vector
 * \param definition
 * \note don't forget to remove the first argv[0] argument
 * \warning the argc and argv are modified in place to remove parsed options
 * \throw MissingValue
 * \throw InvalidOption
 */
IRCCD_EXPORT Result read(int &argc, char **&argv, const Options &definition);

} // !option

} // !irccd

#endif // !IRCCD_OPTIONS_HPP

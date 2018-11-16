/*
 * options.hpp -- parse Unix command line options
 *
 * Copyright (c) 2015-2018 David Demelier <markand@malikania.fr>
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

namespace irccd {

/**
 * Namespace for options parsing.
 */
namespace option {

/**
 * \brief This exception is thrown when an invalid option has been found.
 */
class invalid_option : public std::exception {
private:
    std::string message_;
    std::string name_;

public:
    /**
     * Construct the exception.
     *
     * \param name the argument missing
     */
    inline invalid_option(std::string name)
        : name_(std::move(name))
    {
        message_ = std::string("invalid option: ") + name_;
    }

    /**
     * Get the option name.
     *
     * \return the name
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }

    /**
     * Get the error message.
     *
     * \return the error message
     */
    const char* what() const noexcept override
    {
        return message_.c_str();
    }
};

/**
 * \brief This exception is thrown when an option requires a value and no value
 * has been given.
 */
class missing_value : public std::exception {
private:
    std::string message_;
    std::string name_;

public:
    /**
     * Construct the exception.
     *
     * \param name the option that requires a value
     */
    inline missing_value(std::string name)
        : name_(std::move(name))
    {
        message_ = std::string("missing argument for: ") + name_;
    }

    /**
     * Get the option name.
     *
     * \return the name
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }

    /**
     * Get the error message.
     *
     * \return the error message
     */
    const char* what() const noexcept override
    {
        return message_.c_str();
    }
};

/**
 * Packed multimap of options.
 */
using result = std::multimap<std::string, std::string>;

/**
 * Define the allowed options.
 */
using options = std::map<std::string, bool>;

/**
 * Extract the command line options and return a result.
 *
 * \param args the arguments
 * \param definition
 * \warning the arguments vector is modified in place to remove parsed options
 * \throw missing_value
 * \throw invalid_option
 * \return the result
 */
result read(std::vector<std::string>& args, const options& definition);

/**
 * Overloaded function for usage with main() arguments.
 *
 * \param argc the number of arguments
 * \param argv the argument vector
 * \param definition
 * \note don't forget to remove the first argv[0] argument
 * \warning the argc and argv are modified in place to remove parsed options
 * \throw missing_value
 * \throw invalid_option
 * \return the result
 */
result read(int& argc, char**& argv, const options& definition);

} // !option

} // !irccd

#endif // !IRCCD_OPTIONS_HPP

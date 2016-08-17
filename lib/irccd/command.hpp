/*
 * command.hpp -- remote command
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

#ifndef IRCCD_COMMAND_HPP
#define IRCCD_COMMAND_HPP

/**
 * \file command.hpp
 * \brief Remote commands.
 */

#include <cassert>
#include <map>
#include <vector>

#include "json.hpp"
#include "sysconfig.hpp"

namespace irccd {

class Irccd;
class Irccdctl;

/**
 * \brief A JSON property is missing.
 */
class MissingPropertyError : public std::exception {
private:
    std::string m_message;
    std::string m_name;
    std::vector<nlohmann::json::value_t> m_types;

public:
    /**
     * Constructor.
     */
    MissingPropertyError(std::string name, std::vector<nlohmann::json::value_t> types);

    /**
     * Get human error message.
     *
     * \return a message
     */
    const char *what() const noexcept override
    {
        return m_message.c_str();
    }
};

/**
 * \brief A JSON property is invalid
 */
class InvalidPropertyError : public std::exception {
private:
    std::string m_message;
    std::string m_name;

    nlohmann::json::value_t m_expected;
    nlohmann::json::value_t m_result;

public:
    /**
     * Constructor.
     *
     * \param name the property name
     * \param expected the expected type
     * \param result the type received
     */
    InvalidPropertyError(std::string name, nlohmann::json::value_t expected, nlohmann::json::value_t result);

    /**
     * Get human error message.
     *
     * \return a message
     */
    const char *what() const noexcept override
    {
        return m_message.c_str();
    }
};

/**
 * \brief Property range error.
 */
class PropertyRangeError : public std::exception {
private:
    std::string m_message;
    std::string m_name;
    std::uint64_t m_min;
    std::uint64_t m_max;
    std::uint64_t m_value;

public:
    /**
     * Constructor.
     *
     * \pre value < min || value > max
     * \param name the property name
     * \param min the minimum value
     * \param max the maximum value
     * \param value the actual value
     */
    PropertyRangeError(std::string name, std::uint64_t min, std::uint64_t max, std::uint64_t value);

    /**
     * Get human error message.
     *
     * \return a message
     */
    const char *what() const noexcept override
    {
        return m_message.c_str();
    }
};

/**
 * \brief Generic error for JSON properties.
 */
class PropertyError : public std::exception {
private:
    std::string m_message;
    std::string m_name;

public:
    /**
     * Constructor.
     *
     * \param name the property name
     * \param message the error message
     */
    PropertyError(std::string name, std::string message);

    /**
     * Get human error message.
     *
     * \return a message
     */
    const char *what() const noexcept override
    {
        return m_message.c_str();
    }
};


/**
 * \brief Namespace for remote commands.
 */
//namespace command {

/**
 * \brief Command line arguments to irccdctl.
 *
 * This class contains the resolved arguments from command line that can apply
 * to the command.
 */
class CommandRequest {
public:
    /**
     * The options given by command line.
     */
    using Options = std::multimap<std::string, std::string>;

    /**
     * Command line arguments in the same order.
     */
    using Args = std::vector<std::string>;

private:
    Options m_options;
    Args m_args;

public:
    /**
     * Construct the request.
     *
     * \param options the options
     * \param args the arguments
     */
    inline CommandRequest(Options options, Args args) noexcept
        : m_options(std::move(options))
        , m_args(std::move(args))
    {
    }

    /**
     * Get the arguments.
     *
     * \return the arguments
     */
    inline const Args &args() const noexcept
    {
        return m_args;
    }

    /**
     * Get the options.
     *
     * \return the options
     */
    inline const Options &options() const noexcept
    {
        return m_options;
    }

    /**
     * Get the number of arguments.
     *
     * \return the number of arguments
     */
    inline unsigned length() const noexcept
    {
        return (unsigned)m_args.size();
    }

    /**
     * Check if the request has the given option id.
     *
     * \param option the option id
     * \return true if the option is available
     */
    inline bool has(const std::string &option) const noexcept
    {
        return m_options.count(option) != 0;
    }

    /**
     * Get the argument at the specified index.
     *
     * \pre index < length()
     * \param index the argument index
     * \return the argument
     */
    inline const std::string &arg(unsigned index) const noexcept
    {
        assert(index < m_args.size());

        return m_args[index];
    }

    /**
     * Get the argument or default value if not available.
     *
     * \param index the index
     * \param defaultValue the value if index is out of range
     * \return the argument
     */
    inline std::string argOr(unsigned index, std::string defaultValue) const noexcept
    {
        return index < m_args.size() ? m_args[index] : defaultValue;
    }

    /**
     * Get the given option by its id.
     *
     * \pre has(key)
     * \param key the option id
     * \return the option
     */
    inline const std::string &option(const std::string &key) const noexcept
    {
        assert(m_options.count(key) != 0);

        return m_options.find(key)->second;
    }

    /**
     * Get the given option by its id or defaultValue if not found.
     *
     * \param key the option id
     * \param defaultValue the value replacement
     * \return the option
     */
    inline std::string optionOr(const std::string &key, std::string defaultValue) const noexcept
    {
        auto it = m_options.find(key);

        if (it == m_options.end())
            return defaultValue;

        return it->second;
    }
};

/**
 * \brief Invokable command.
 *
 * A remote command is a invokable command in the irccd daemon. You can register
 * dynamically any remote command you like using Application::addCommand.
 *
 * The remote command will be usable directly from irccdctl without any other
 * code.
 *
 * A remote command can have options and arguments. Options always come first,
 * before arguments.
 *
 * The command workflow is defined as follow:
 *
 * 1. User wants to invoke a command, request() is called and return a JSON
 *    object containaing the request, it it send to the daemon.
 *
 * 2. The daemon receive the request and execute it using exec(). It returns a
 *    JSON object containint the request result or error if any.
 *
 * 3. Finally, the command receives the result in result() function and user can
 *    manipulate it. For convenience, the default implementation shows the error
 *    if any.
 */
class Command {
public:
    /**
     * \brief Defines available options for this command.
     */
    class Option;

    /**
     * \brief Defines available arguments for this command.
     */
    class Arg;

    /**
     * \brief Defines properties that must be available in the JSON request.
     */
    class Property;

private:
    std::string m_name;
    std::string m_category;
    std::string m_description;
    bool m_visible;

public:
    /**
     * Create the remote command.
     *
     * \pre name must not be empty
     * \pre category must not be empty
     * \param name the command name (e.g. server-list)
     * \param category the category (e.g. Server)
     * \param description a one line description with no dots, no new line
     * \param visible true if the command should be visible without verbosity
     */
    inline Command(std::string name,
                   std::string category,
                   std::string description,
                   bool visible = true) noexcept
        : m_name(std::move(name))
        , m_category(std::move(category))
        , m_description(std::move(description))
        , m_visible(visible)
    {
        assert(!m_name.empty());
        assert(!m_category.empty());
    }

    /**
     * Default destructor virtual.
     */
    virtual ~Command() = default;

    /**
     * Return the command name, must not have spaces.
     *
     * \return the command name
     */
    inline const std::string &name() const noexcept
    {
        return m_name;
    }

    /**
     * Get the command category.
     *
     * Irccdctl will sort commands by categories.
     *
     * \return the category
     */
    inline const std::string &category() const noexcept
    {
        return m_category;
    }

    /**
     * Get the command description.
     *
     * \return the description
     */
    inline const std::string &description() const noexcept
    {
        return m_description;
    }

    /**
     * Hide the command in non-verbose mode.
     *
     * \return true if the command should be visible in non-verbose mode
     */
    inline bool visible() const noexcept
    {
        return m_visible;
    }

    /**
     * Return the command documentation usage.
     *
     * \return the usage
     */
    IRCCD_EXPORT std::string usage() const;

    /**
     * Return the help message.
     *
     * \return the help message
     */
    IRCCD_EXPORT std::string help() const;

    /**
     * Get the supported irccdctl options.
     *
     * \return the options
     */
    virtual std::vector<Option> options() const
    {
        return {};
    }

    /**
     * Get the supported arguments.
     *
     * \return the arguments
     */
    virtual std::vector<Arg> args() const
    {
        return {};
    }

    /**
     * Get the properties required in the JSON request.
     *
     * Default implementation returns empty list.
     *
     * \return the required properties
     * \note Put only **required** properties
     */
    virtual std::vector<Property> properties() const
    {
        return {};
    }

    /**
     * Get the minimum number of arguments required.
     *
     * \return the minimum
     */
    IRCCD_EXPORT unsigned min() const noexcept;

    /**
     * Get the maximum number of arguments required.
     *
     * \return the maximum
     */
    IRCCD_EXPORT unsigned max() const noexcept;

    /**
     * Prepare a JSON request to the daemon.
     *
     * If the command is local and does not need to send anything to irccd's
     * instance, return a null JSON value.
     *
     * The default implementation just send the command name with no arguments.
     *
     * \param irccdctl the irccdctl instance
     * \param args the command line arguments and options
     * \return the JSON object to send to the daemon
     * \post the returned JSON value must be an object
     */
    IRCCD_EXPORT virtual nlohmann::json request(Irccdctl &irccdctl, const CommandRequest &args) const;

    /**
     * Execute the command in the daemon.
     *
     * The user can return an object with any properties to forward to the
     * client. Irccd will automatically add the command name and the appropriate
     * status code.
     *
     * The default return an empty object which indicates success.
     *
     * If any exception is thrown from this function, it is forwarded to the
     * client as error status.
     *
     * \param irccd the instance
     * \param request the JSON request
     * \return the response
     */
    IRCCD_EXPORT virtual nlohmann::json exec(Irccd &irccd, const nlohmann::json &request) const;

    /**
     * What to do when receiving the response from irccd.
     *
     * This default implementation just check for an error string and shows it
     * if any.
     *
     * \param irccdctl the irccdctl instance
     * \param response the JSON response
     */
    IRCCD_EXPORT virtual void result(Irccdctl &irccdctl, const nlohmann::json &response) const;
};

/**
 * \brief Option description for a command.
 */
class Command::Option {
private:
    std::string m_id;
    std::string m_simple;
    std::string m_long;
    std::string m_arg;
    std::string m_description;

public:
    /**
     * Constructor an option description.
     *
     * Simple and long keys must not start with '-' or '--', they will be added
     * automatically.
     *
     * If arg is not empty, the option takes an argument.
     *
     * \pre id must not be empty
     * \pre at least simpleKey or longKey must not be empty
     * \pre description must not be empty
     * \param id the option id
     * \param simpleKey the key the option key
     * \param longKey the long option name
     * \param arg the argument name if needed
     * \param description the description
     */
    inline Option(std::string id,
              std::string simpleKey,
              std::string longKey,
              std::string arg,
              std::string description) noexcept
        : m_id(std::move(id))
        , m_simple(std::move(simpleKey))
        , m_long(std::move(longKey))
        , m_arg(std::move(arg))
        , m_description(std::move(description))
    {
        assert(!m_id.empty());
        assert(!m_simple.empty() || !m_long.empty());
        assert(!m_description.empty());
    }

    /**
     * Get the id.
     *
     * \return the id
     */
    inline const std::string &id() const noexcept
    {
        return m_id;
    }

    /**
     * Get the option key.
     *
     * \return the key
     */
    inline const std::string &simpleKey() const noexcept
    {
        return m_simple;
    }

    /**
     * Get the long option.
     *
     * \return the long option
     */
    inline const std::string &longKey() const noexcept
    {
        return m_long;
    }

    /**
     * Get the option description.
     *
     * \return the description
     */
    inline const std::string &description() const noexcept
    {
        return m_description;
    }

    /**
     * Get the option argument name.
     *
     * \return the argument name if any
     */
    inline const std::string &arg() const noexcept
    {
        return m_arg;
    }
};

/**
 * \brief Argument description for command.
 */
class Command::Arg {
private:
    std::string m_name;
    bool m_required;

public:
    /**
     * Construct an argument.
     *
     * \param name the name
     * \param required true if the argument is required
     */
    inline Arg(std::string name, bool required) noexcept
        : m_name(std::move(name))
        , m_required(required)
    {
    }

    /**
     * Get the argument name.
     *
     * \return the name
     */
    inline const std::string &name() const noexcept
    {
        return m_name;
    }

    /**
     * Tells if the argument is required.
     *
     * \return true if required
     */
    inline bool required() const noexcept
    {
        return m_required;
    }
};

/**
 * \brief Property description for JSON request.
 */
class Command::Property {
private:
    std::string m_name;
    std::vector<nlohmann::json::value_t> m_types;

public:
    /**
     * Construct the property description.
     *
     * \pre !name.empty()
     * \pre types.size() >= 1
     * \param name the name
     * \param types the json types allowed
     */
    inline Property(std::string name, std::vector<nlohmann::json::value_t> types = { nlohmann::json::value_t::string }) noexcept
        : m_name(std::move(name))
        , m_types(std::move(types))
    {
        assert(!m_name.empty());
        assert(m_types.size() >= 1);
    }

    /**
     * Get the property name.
     *
     * \return the name
     */
    inline const std::string &name() const noexcept
    {
        return m_name;
    }

    /**
     * Get the property types.
     *
     * \return the types
     */
    inline const std::vector<nlohmann::json::value_t> &types() const noexcept
    {
        return m_types;
    }
};

} // !irccd

#endif // !IRCCD_COMMAND_HPP

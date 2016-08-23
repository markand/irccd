/*
 * irccdctl.hpp -- main irccdctl class
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

#ifndef IRCCD_IRCCDCTL_HPP
#define IRCCD_IRCCDCTL_HPP

/**
 * \file irccdctl.hpp
 * \brief Base class for irccdctl front end.
 */

#include <map>
#include <memory>
#include <string>

#include "client.hpp"
#include "alias.hpp"
#include "options.hpp"
#include "service-command.hpp"

#include <json.hpp>

namespace irccd {

class Client;

namespace ini {

class Document;
class Section;

} // !ini

/**
 * \brief Main irccdctl class.
 */
class Irccdctl {
private:
    // Commands.
    CommandService m_commandService;

    // Connection handler.
    std::unique_ptr<Client> m_connection;
    std::uint32_t m_timeout{30000};
    net::Address m_address;

    // Aliases.
    std::map<std::string, Alias> m_aliases;

    // Incoming data.
    std::vector<nlohmann::json> m_input;

    void usage() const;
    void help() const;

    // Parse configuration file.
    void readConnectIp(const ini::Section &sc);
    void readConnectLocal(const ini::Section &sc);
    void readConnect(const ini::Section &sc);
    void readGeneral(const ini::Section &sc);
    void readAliases(const ini::Section &sc);
    void read(const std::string &path);

    // Parse command line options.
    void parseConnectIp(const option::Result &options);
    void parseConnectLocal(const option::Result &options);
    void parseConnect(const option::Result &options);
    option::Result parse(int &argc, char **&argv);

public:
    /**
     * Get the command service.
     *
     * \return the command service
     */
    inline CommandService &commandService() noexcept
    {
        return m_commandService;
    }

    /**
     * Get the client connection to irccd.
     *
     * \return the connection
     */
    inline const Client &client() const noexcept
    {
        return *m_connection;
    }

    /**
     * Get the client connection to irccd.
     *
     * \return the connection
     */
    inline Client &client() noexcept
    {
        return *m_connection;
    }

    /**
     * Get the next response with the given id.
     *
     * If the response id is not provided, get the next incoming message.
     *
     * Otherwise, if the id is provided, all other previous messages will be
     * discarded.
     *
     * \param id the response id (e.g. server-message)
     * \return the next message
     * \warning this may skip previous events
     */
    IRCCD_EXPORT nlohmann::json next(const std::string id = "");

    /**
     * Execute the given command and wait for its result.
     *
     * \param cmd the command
     * \param args the arguments
     */
    IRCCD_EXPORT void exec(const Command &cmd, std::vector<std::string> args);

    /**
     * Execute the given alias.
     *
     * \param alias the alias
     * \param args the arguments
     */
    IRCCD_EXPORT void exec(const Alias &alias, std::vector<std::string> args);

    /**
     * Resolve the command line arguments.
     *
     * \param args the main arguments
     */
    IRCCD_EXPORT void exec(std::vector<std::string> args);

    /**
     * Run the irccdctl front end.
     *
     * \param argc the number of arguments
     * \param argv the arguments
     */
    IRCCD_EXPORT void run(int argc, char **argv);
};

} // !irccd

#endif // !IRCCD_IRCCDCTL_HPP

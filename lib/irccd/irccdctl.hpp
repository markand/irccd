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

#include <cassert>
#include <map>
#include <memory>
#include <string>

#include "alias.hpp"
#include "connection.hpp"
#include "options.hpp"
#include "service-command.hpp"

namespace irccd {

class Command;

namespace ini {

class Section;

} // !ini

/**
 * \brief Main irccdctl class.
 */
class Irccdctl {
private:
    // Irccd's information.
    unsigned short m_major{0};
    unsigned short m_minor{0};
    unsigned short m_patch{0};

    // Irccd's compilation option.
    bool m_javascript{true};
    bool m_ssl{true};

    // Commands.
    CommandService m_commandService;

    std::unique_ptr<Connection> m_connection;
    std::unordered_map<std::string, Alias> m_aliases;

    void usage() const;

    void readConnectIp(const ini::Section &sc);
    void readConnectUnix(const ini::Section &sc);
    void readConnect(const ini::Section &sc);
    void readGeneral(const ini::Section &sc);
    void readAliases(const ini::Section &sc);
    void read(const std::string &path, const option::Result &options);

    void parseConnectIp(const option::Result &options, bool ipv6);
    void parseConnectUnix(const option::Result &options);
    void parseConnect(const option::Result &options);
    option::Result parse(int &argc, char **&argv) const;

    void connect();

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
     * Get the connection.
     *
     * \return the connection
     */
    inline Connection &connection() noexcept
    {
        return *m_connection;
    }

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

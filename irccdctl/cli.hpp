/*
 * cli.hpp -- command line for irccdctl
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

#ifndef IRCCD_CLI_HPP
#define IRCCD_CLI_HPP

#include <string>
#include <vector>

#include <json.hpp>

namespace irccd {

class Irccdctl;

class Cli {
protected:
    std::string m_name;
    std::string m_summary;
    std::string m_usage;
    std::string m_help;

    /**
     * Check the message result and print an error if any.
     *
     * \param result the result
     * \throw an error if any
     */
    void check(const nlohmann::json &result);

    /**
     * Send a request and wait for the response.
     *
     * \param irccdctl the client
     * \param args the optional arguments
     */
    nlohmann::json request(Irccdctl &irccdctl, nlohmann::json args = nullptr);

    /**
     * Call a command and wait for success/error response.
     *
     * \param irccdctl the client
     * \param args the extra arguments (may be null)
     */
    void call(Irccdctl &irccdctl, nlohmann::json args);

public:
    inline Cli(std::string name, std::string summary, std::string usage, std::string help) noexcept
        : m_name(std::move(name))
        , m_summary(std::move(summary))
        , m_usage(std::move(usage))
        , m_help(std::move(help))
    {
    }

    virtual ~Cli() = default;

    inline const std::string &name() const noexcept
    {
        return m_name;
    }

    inline const std::string &summary() const noexcept
    {
        return m_summary;
    }

    inline const std::string &usage() const noexcept
    {
        return m_usage;
    }

    inline const std::string &help() const noexcept
    {
        return m_help;
    }

    virtual void exec(Irccdctl &client, const std::vector<std::string> &args) = 0;
};

} // !irccd

#endif // !IRCCD_CLI_HPP

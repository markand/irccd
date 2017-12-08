/*
 * cli.hpp -- command line for irccdctl
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

#ifndef IRCCD_CTL_CLI_HPP
#define IRCCD_CTL_CLI_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include <json.hpp>

namespace irccd {

namespace ctl {

class controller;

/**
 * \brief Abstract CLI class.
 */
class cli {
public:
    /**
     * Convenient handler for request function.
     */
    using handler_t = std::function<void (nlohmann::json)>;

private:
    void recv_response(ctl::controller&, nlohmann::json, handler_t);

protected:
    /**
     * Convenient request helper.
     *
     * This function send and receive the response for the given request. It
     * checks for an error code or string in the command result and throws it if
     * any.
     *
     * If handler is not null, it will be called once the command result has
     * been received.
     *
     * This function may executes successive read calls until we get the
     * response.
     *
     * \param ctl the controller
     * \param json the json object
     * \param handler the optional handler
     */
    void request(ctl::controller& ctl, nlohmann::json json, handler_t handler = nullptr);

public:
    /**
     * Default constructor.
     */
    cli() noexcept = default;

    /**
     * Virtual destructor defaulted.
     */
    virtual ~cli() noexcept = default;

    /**
     * Return the command name.
     *
     * \return the name
     */
    virtual std::string name() const = 0;

    /**
     * Execute the command.
     *
     * \param ctl the controller
     * \param args the user arguments
     */
    virtual void exec(ctl::controller& ctl, const std::vector<std::string>& args) = 0;
};

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_CLI_HPP

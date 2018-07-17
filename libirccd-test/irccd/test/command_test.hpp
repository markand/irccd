/*
 * command_test.hpp -- test fixture helper for transport commands
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

#ifndef IRCCD_TEST_COMMAND_TEST_HPP
#define IRCCD_TEST_COMMAND_TEST_HPP

#include <memory>

#include <irccd/socket_acceptor.hpp>
#include <irccd/socket_connector.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/service/transport_service.hpp>

#include <irccd/ctl/controller.hpp>

namespace irccd {

template <typename... Commands>
class command_test {
private:
    template <typename Command>
    inline void add()
    {
        daemon_->transports().get_commands().push_back(std::make_unique<Command>());
    }

    template <typename C1, typename C2, typename... Tail>
    inline void add()
    {
        add<C1>();
        add<C2, Tail...>();
    }

protected:
    /**
     * Result for request function.
     */
    using result = std::pair<nlohmann::json, std::error_code>;

    boost::asio::io_service service_;

    // daemon stuff.
    std::unique_ptr<irccd> daemon_;

    // controller stuff.
    std::unique_ptr<ctl::controller> ctl_;

    command_test();

    template <typename Condition>
    void wait_for(Condition&& cond)
    {
        service_.reset();

        while (!cond())
            service_.poll();
    }

    result request(nlohmann::json json)
    {
        result r;

        ctl_->write(std::move(json));
        ctl_->read([&] (auto result, auto message) {
            r.first = message;
            r.second = result;
        });
        wait_for([&] {
            return r.second || r.first.is_object();
        });

        return r;
    }
};

template <typename... Commands>
command_test<Commands...>::command_test()
    : daemon_(std::make_unique<irccd>(service_))
{
    using boost::asio::ip::tcp;

    // Bind to a random port.
    tcp::endpoint ep(tcp::v4(), 0);
    tcp::acceptor acc(service_, ep);

    // Create controller and transport server.
    ctl_ = std::make_unique<ctl::controller>(
        std::make_unique<io::ip_connector>(service_, acc.local_endpoint()));
    daemon_->transports().add(std::make_unique<transport_server>(
        std::make_unique<io::ip_acceptor>(std::move(acc))));

    // Add the server and the command.
    add<Commands...>();
    daemon_->set_log(std::make_unique<logger::silent_sink>());

    // Wait for controller to connect.
    boost::asio::deadline_timer timer(service_);

    timer.expires_from_now(boost::posix_time::seconds(10));
    timer.async_wait([] (auto code) {
        if (code && code != boost::asio::error::operation_aborted)
            throw std::system_error(make_error_code(std::errc::timed_out));
    });

    bool connected = false;

    ctl_->connect([&] (auto code, auto) {
        timer.cancel();

        if (code)
            throw std::system_error(code);

        connected = true;
    });

    /**
     * Irccd will block indefinitely since transport_service will wait for any
     * new client again, so we need to check with a boolean.
     */
    while (!connected)
        service_.poll();
}

} // !irccd

#endif // !IRCCD_TEST_COMMAND_TEST_HPP

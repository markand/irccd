/*
 * command_test.hpp -- test fixture helper for transport commands
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

#ifndef IRCCD_TEST_COMMAND_TEST_HPP
#define IRCCD_TEST_COMMAND_TEST_HPP

#include <memory>

#include <irccd/logger.hpp>

#include <irccd/command_service.hpp>
#include <irccd/ip_transport_server.hpp>
#include <irccd/irccd.hpp>
#include <irccd/transport_service.hpp>

#include <irccd/ctl/ip_connection.hpp>
#include <irccd/ctl/controller.hpp>

namespace irccd {

template <typename... Commands>
class command_test {
private:
    template <typename Command>
    inline void add()
    {
        daemon_->commands().add(std::make_unique<Command>());
    }

    template <typename C1, typename C2, typename... Tail>
    inline void add()
    {
        add<C1>();
        add<C2, Tail...>();
    }

protected:
    boost::asio::io_service service_;
    boost::asio::deadline_timer timer_;

    // daemon stuff.
    std::unique_ptr<irccd> daemon_;

    // controller stuff.
    std::unique_ptr<ctl::connection> conn_;
    std::unique_ptr<ctl::controller> ctl_;

    command_test();

    template <typename Condition>
    void wait_for(Condition&& cond)
    {
        service_.reset();

        while (!cond())
            service_.poll();
    }
};

template <typename... Commands>
command_test<Commands...>::command_test()
    : timer_(service_)
    , daemon_(std::make_unique<irccd>(service_))
{
    using boost::asio::ip::tcp;

    log::set_logger(std::make_unique<log::silent_logger>());

    // Bind to a random port.
    tcp::endpoint ep(tcp::v4(), 0);
    tcp::acceptor acc(service_, ep);

    // Connect to the local bound port.
    conn_ = std::make_unique<ctl::ip_connection>(service_, "127.0.0.1", acc.local_endpoint().port());
    ctl_ = std::make_unique<ctl::controller>(*conn_);

    // Add the server and the command.
    add<Commands...>();
    daemon_->transports().add(std::make_unique<ip_transport_server>(std::move(acc)));

    timer_.expires_from_now(boost::posix_time::seconds(10));
    timer_.async_wait([] (auto code) {
        if (!code)
            throw make_error_code(boost::system::errc::timed_out);
    });

    bool connected = false;

    ctl_->connect([&] (auto code, auto) {
        if (code)
            throw code;

        connected = true;
        timer_.cancel();
    });

    while (!connected)
        service_.poll();

    service_.reset();

    if (!connected)
        throw std::runtime_error("unable to connect");
}

} // !irccd

#endif // !IRCCD_TEST_COMMAND_TEST_HPP

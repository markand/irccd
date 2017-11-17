/*
 * irccd.cpp -- main irccd class
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

#include "irccd.hpp"
#include "logger.hpp"
#include "net_util.hpp"
#include "transport_service.hpp"
#include "service.hpp"
#include "util.hpp"

namespace irccd {

irccd::irccd(boost::asio::io_service& service, std::string config)
    : config_(std::move(config))
    , service_(service)
    , command_service_(std::make_unique<command_service>())
    , itr_service_(std::make_unique<interrupt_service>())
    , server_service_(std::make_unique<server_service>(*this))
    , tpt_service_(std::make_unique<transport_service>(*this))
    , rule_service_(std::make_unique<rule_service>())
    , plugin_service_(std::make_unique<plugin_service>(*this))
{
}

irccd::~irccd() = default;

void irccd::post(std::function<void (irccd&)> ev) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);

    events_.push_back(std::move(ev));
    itr_service_->interrupt();
}

void irccd::run()
{
    while (running_) {
        net_util::poll(100, *this);
        service_.poll();
    }
}

void irccd::prepare(fd_set& in, fd_set& out, net::Handle& max)
{
    net_util::prepare(in, out, max, *itr_service_, *server_service_);
}

void irccd::sync(fd_set& in, fd_set& out)
{
    if (!running_)
        return;

    net_util::sync(in, out, *itr_service_, *server_service_);

    if (!running_)
        return;

    /*
     * Make a copy because the events can add other events while we are
     * iterating it. Also lock because the timers may alter these events too.
     */
    std::vector<std::function<void (irccd&)>> copy;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        copy = std::move(events_);
        events_.clear();
    }

    if (copy.size() > 0)
        log::debug() << "irccd: dispatching " << copy.size() << " event"
                     << (copy.size() > 1 ? "s" : "") << std::endl;

    for (auto& ev : copy)
        ev(*this);
}

void irccd::stop()
{
    log::debug() << "irccd: requesting to stop now" << std::endl;

    running_ = false;
    itr_service_->interrupt();
}

} // !irccd

/*
 * service-interrupt.cpp -- interrupt irccd event loop
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

#include <array>

#include "logger.hpp"
#include "service-interrupt.hpp"

namespace irccd {

InterruptService::InterruptService()
    : m_in(AF_INET, 0)
    , m_out(AF_INET, 0)
{
    // Bind a socket to any port.
    m_in.set(net::option::SockReuseAddress(true));
    m_in.bind(net::ipv4::any(0));
    m_in.listen(1);

    // Do the socket pair.
    m_out.connect(net::ipv4::pton("127.0.0.1", net::ipv4::port(m_in.getsockname())));
    m_in = m_in.accept();
    m_out.set(net::option::SockBlockMode(false));
}

void InterruptService::prepare(fd_set &in, fd_set &, net::Handle &max)
{
    FD_SET(m_in.handle(), &in);

    if (m_in.handle() > max)
        max = m_in.handle();
}

void InterruptService::sync(fd_set &in, fd_set &)
{
    if (FD_ISSET(m_in.handle(), &in)) {
        static std::array<char, 32> tmp;

        try {
            log::debug("irccd: interrupt service recv");
            m_in.recv(tmp.data(), 32);
        } catch (const std::exception &ex) {
            log::warning() << "irccd: interrupt service error: " << ex.what() << std::endl;
        }
    }
}

void InterruptService::interrupt() noexcept
{
    try {
        static char byte;

        log::debug("irccd: interrupt service send");
        m_out.send(&byte, 1);
    } catch (const std::exception &ex) {
        log::warning() << "irccd: interrupt service error: " << ex.what() << std::endl;
    }
}

} // !irccd

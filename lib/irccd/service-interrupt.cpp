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

#include "logger.hpp"
#include "service-interrupt.hpp"

namespace irccd {

InterruptService::InterruptService()
{
	// Bind a socket to any port.
	m_in.set(net::option::SockReuseAddress(true));
	m_in.bind(net::address::Ipv4("*", 0));
	m_in.listen(1);

	// Do the socket pair.
	m_out.connect(net::address::Ipv4("127.0.0.1", m_in.getsockname().port()));
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
		try {
			log::debug("irccd: interrupt service recv");
			m_in.recv(32);
		} catch (const std::exception &ex) {
			log::warning() << "irccd: interrupt service error: " << ex.what() << std::endl;
		}
	}
}

void InterruptService::interrupt() noexcept
{
	try {
		log::debug("irccd: interrupt service send");
		m_out.send(" ");
	} catch (const std::exception &ex) {
		log::warning() << "irccd: interrupt service error: " << ex.what() << std::endl;
	}
}

} // !irccd

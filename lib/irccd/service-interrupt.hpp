/*
 * service-interrupt.hpp -- interrupt irccd event loop
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

#ifndef IRCCD_SERVICE_INTERRUPT_HPP
#define IRCCD_SERVICE_INTERRUPT_HPP

/**
 * \file service-interrupt.hpp
 * \brief Interrupt irccd event loop.
 */

#include "service.hpp"

namespace irccd {

/**
 * \brief Interrupt irccd event loop.
 * \ingroup services
 */
class InterruptService : public Service {
private:
    net::TcpSocket m_in;
    net::TcpSocket m_out;

public:
    /**
     * Prepare the socket pair.
     *
     * \throw std::runtime_error on errors
     */
    IRCCD_EXPORT InterruptService();

    /**
     * \copydoc Service::prepare
     */
    IRCCD_EXPORT void prepare(fd_set &in, fd_set &out, net::Handle &max) override;

    /**
     * \copydoc Service::sync
     */
    IRCCD_EXPORT void sync(fd_set &in, fd_set &out) override;

    /**
     * Request interruption.
     */
    IRCCD_EXPORT void interrupt() noexcept;
};

} // !irccd

#endif // !IRCCD_SERVICE_INTERRUPT_HPP

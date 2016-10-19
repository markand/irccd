/*
 * server-tester.hpp -- server that does nothing
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

#ifndef IRCCD_SERVER_TESTER_HPP
#define IRCCD_SERVER_TESTER_HPP

/**
 * \file server-tester.hpp
 * \brief Server that does nothing.
 */

#include "server.hpp"
#include "sysconfig.hpp"

namespace irccd {

/**
 * \brief Useless server for testing purpose.
 */
class IRCCD_EXPORT ServerTester : public Server {
public:
    /**
     * Create a server with named 'test' by default.
     */
    ServerTester(std::string name = "test");

    /**
     * Overload that is a no-op.
     */
    void prepare(fd_set &, fd_set &, net::Handle &) noexcept override;

    /**
     * Overload that is a no-op.
     */
    void sync(fd_set &, fd_set &) override;
};

} // !irccd

#endif // !IRCCD_SERVER_TESTER_HPP

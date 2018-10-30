/*
 * acceptor.hpp -- abstract stream acceptor interface
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

#ifndef IRCCD_ACCEPTOR_HPP
#define IRCCD_ACCEPTOR_HPP

/**
 * \file acceptor.hpp
 * \brief Abstract stream acceptor interface.
 */

#include <functional>
#include <memory>
#include <system_error>

namespace irccd {

class stream;

/**
 * \brief Abstract stream acceptor interface.
 *
 * This class is used to wait a new client in an asynchronous manner. Derived
 * classes must implement a non-blocking accept function.
 */
class acceptor {
public:
	/**
	 * \brief Accept completion handler.
	 */
	using handler = std::function<void (std::error_code, std::shared_ptr<stream>)>;

public:
	/**
	 * Default constructor.
	 */
	acceptor() = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~acceptor() = default;

	/**
	 * Start asynchronous accept.
	 *
	 * Once the client is accepted, the original acceptor must be kept until it
	 * is destroyed.
	 *
	 * \pre another accept operation must not be running
	 * \pre handler != nullptr
	 * \param handler the handler
	 */
	virtual void accept(handler handler) = 0;
};

} // !irccd

#endif // !IRCCD_ACCEPTOR_HPP

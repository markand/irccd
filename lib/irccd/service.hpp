/*
 * service.hpp -- selectable service
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

#ifndef IRCCD_SERVICE_HPP
#define IRCCD_SERVICE_HPP

/**
 * \file service.hpp
 * \brief Selectable service.
 */

#include "sockets.hpp"

namespace irccd {

/**
 * \brief Selectable service.
 *
 * This class can be used to prepare a set of sockets that will be selected by Irccd class.
 *
 * First, the function prepare is called, the user is responsible to fill the input and output set and adjust max
 * accordingly.
 *
 * Second, after select has been called, sync is called. The user is responsible of checking which sockets are ready
 * for input or output.
 */
class Service {
public:
	/**
	 * Default constructor.
	 */
	Service() noexcept = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~Service() noexcept = default;

	/**
	 * Prepare the input and output set.
	 *
	 * \param in the input set
	 * \param out the output set
	 * \param max the handle to update
	 */
	virtual void prepare(fd_set &in, fd_set &out, net::Handle &max)
	{
		(void)in;
		(void)out;
		(void)max;
	}

	/**
	 * Synchronize with result sets.
	 *
	 * \param in the input set
	 * \param out the output set
	 */
	virtual void sync(fd_set &in, fd_set &out)
	{
		(void)in;
		(void)out;
	}
};

} // !irccd

#endif // !IRCCD_SERVICE_HPP
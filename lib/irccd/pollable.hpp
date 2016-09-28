/*
 * pollable.hpp -- pollable object
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

#ifndef IRCCD_POLLABLE_HPP
#define IRCCD_POLLABLE_HPP

/**
 * \file service.hpp
 * \brief Pollable object.
 */

#include "net.hpp"
#include "util.hpp"

namespace irccd {

/**
 * \brief Pollable object.
 *
 * This class can be used to prepare an object into a select(2) system call.
 *
 * The primary use case of these objects is to be polled in the main loop while
 * being generic.
 *
 * To use the pollable objects:
 *
 * 1. Create two fd_set, one for input and one for output. Don't forget to
 *    initialize them using FD_ZERO.
 *
 * 2. For all of your pollable objects, call the prepare function and pass the
 *    input and output sets. The max handle is usually the pollable socket.
 *
 * 3. Do your select(2) call using the input, output and socket handle and your
 *    desired timeout.
 *
 * 4. For all of your pollable objects, call the sync function and pass the
 *    input and output sets.
 *
 * Pollable objects are usually implemented using asynchronous signals defined
 * in signals.hpp file.
 */
class Pollable {
public:
    /**
     * Default constructor.
     */
    Pollable() noexcept = default;

    /**
     * Virtual destructor defaulted.
     */
    virtual ~Pollable() noexcept = default;

    /**
     * Prepare the input and output set.
     *
     * \param in the input set
     * \param out the output set
     * \param max the handle to update
     */
    virtual void prepare(fd_set &in, fd_set &out, net::Handle &max)
    {
        util::unused(in, out, max);
    }

    /**
     * Synchronize with result sets.
     *
     * \param in the input set
     * \param out the output set
     */
    virtual void sync(fd_set &in, fd_set &out)
    {
        util::unused(in, out);
    }

    /**
     * Convenient function for polling events with a timeout.
     *
     * \param timeout the timeout in milliseconds
     */
    virtual void poll(int timeout = -1)
    {
        fd_set in, out;
        timeval tv = {0, timeout * 1000};

        FD_ZERO(&in);
        FD_ZERO(&out);

        net::Handle max = 0;

        prepare(in, out, max);

        // Timeout or error are discarded.
        if (::select(max + 1, &in, &out, nullptr, timeout < 0 ? nullptr : &tv) > 0)
            sync(in, out);
    }
};

} // !irccd

#endif // !IRCCD_POLLABLE_HPP

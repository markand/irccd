/*
 * stream.hpp -- abstract stream interface
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

#ifndef IRCCD_COMMON_STREAM_HPP
#define IRCCD_COMMON_STREAM_HPP

/**
 * \file stream.hpp
 * \brief Abstract stream interface.
 */

#include <functional>
#include <system_error>

#include "json.hpp"

namespace irccd {

namespace io {

/**
 * \brief Read completion handler.
 */
using read_handler = std::function<void (std::error_code, nlohmann::json)>;

/**
 * \brief Write completion handler.
 */
using write_handler = std::function<void (std::error_code)>;

/**
 * \brief Abstract stream interface
 *
 * Abstract I/O interface that allows reading/writing from a stream in an
 * asynchronous manner.
 *
 * The derived classes must implement non-blocking read and write operations.
 */
class stream {
public:
    /**
     * Default constructor.
     */
    stream() = default;

    /**
     * Virtual destructor defaulted.
     */
    virtual ~stream() = default;

    /**
     * Start asynchronous read.
     *
     * \pre another read operation must not be running
     * \pre handler != nullptr
     * \param handler the handler
     */
    virtual void read(read_handler handler) = 0;

    /**
     * Start asynchronous write.
     *
     * \pre json.is_object()
     * \pre another write operation must not be running
     * \pre handler != nullptr
     * \param json the JSON message
     * \param handler the handler
     */
    virtual void write(const nlohmann::json& json, write_handler handler) = 0;
};

} // !io

} // !irccd

#endif // !IRCCD_COMMON_STREAM_HPP

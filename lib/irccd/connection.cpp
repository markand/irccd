/*
 * connection.cpp -- value wrapper for connecting to irccd
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

#include "connection.hpp"
#include "logger.hpp"

namespace irccd {

json::Value Connection::next(const std::string &name, int timeout)
{
    m_timer.reset();

    for (;;) {
        json::Value object = next(clamp(timeout));

        if (object.isObject() && object["response"].toString() == name)
            return object;
    }

    throw std::runtime_error("connection lost");
}

void Connection::verify(const std::string &name, int timeout)
{
    auto object = next(name, timeout);
    auto value = object.at("status").toString();

    if (!value.empty() && value != "ok")
        throw std::runtime_error(object.at("error").toString());
}

} // !irccd

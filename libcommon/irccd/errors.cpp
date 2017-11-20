/*
 * errors.cpp -- describe some error codes
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

#include "errors.hpp"

namespace irccd {

const boost::system::error_category& network_category() noexcept
{
    static const class network_category : public boost::system::error_category {
    public:
        const char* name() const noexcept override
        {
            return "network_category";
        }

        std::string message(int code) const override
        {
            switch (static_cast<network_error>(code)) {
            case network_error::no_error:
                return "no error";
            case network_error::invalid_program:
                return "invalid program";
            case network_error::invalid_version:
                return "invalid version";
            case network_error::invalid_auth:
                return "invalid authentication";
            case network_error::invalid_message:
                return "invalid message";
            default:
                return "unknown error";
            }
        }
    } category;

    return category;
}

boost::system::error_code make_error_code(network_error errc) noexcept
{
    return {static_cast<int>(errc), network_category()};
}

} // !irccd

/*
 * uri.hpp -- simple uriparser based parser
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

#ifndef IRCCD_URI_HPP
#define IRCCD_URI_HPP

/**
 * \file uri.hpp
 * \brief Simple uriparser based parser.
 */

#include <optional>
#include <string>

namespace irccd {

/**
 * \file uri.hpp
 */
struct uri {
    std::string scheme;     //!< scheme (e.g. http)
    std::string host;       //!< host (e.g. example.org)
    std::string port;       //!< port (e.g. 8080)
    std::string path;       //!< path (e.g. /foo/bar)

    /**
     * Try to parse the uri from the link text.
     *
     * \param link the link
     * \return the uri or nullopt if not parseable
     */
    static auto parse(const std::string& link) noexcept -> std::optional<uri>;
};

} // !irccd

#endif // !IRCCD_URI_HPP

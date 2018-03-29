/*
 * config.hpp -- irccd configuration loader
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

#ifndef IRCCD_COMMON_CONFIG_HPP
#define IRCCD_COMMON_CONFIG_HPP

/**
 * \file config.hpp
 * \brief Read .ini configuration file for irccd
 */

#include <boost/optional.hpp>

#include "ini.hpp"

namespace irccd {

/**
 * \brief Read .ini configuration file for irccd
 */
class config : public ini::document {
private:
    std::string path_;

public:
    /**
     * Search the configuration file into the standard defined paths.
     *
     * \param name the file name
     * \return the config or empty if not found
     */
    static boost::optional<config> search(const std::string& name);

    /**
     * Load the configuration from the specified path.
     *
     * \param path the path
     */
    inline config(std::string path = "")
        : document(path.empty() ? ini::document() : ini::read_file(path))
        , path_(std::move(path))
    {
    }

    /**
     * Get the path to the configuration file.
     *
     * \return the path
     */
    inline const std::string& get_path() const noexcept
    {
        return path_;
    }
};

} // !irccd

#endif // !IRCCD_COMMON_CONFIG_HPP

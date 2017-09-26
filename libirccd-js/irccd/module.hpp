/*
 * module.hpp -- JavaScript API module
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

#ifndef IRCCD_MODULE_HPP
#define IRCCD_MODULE_HPP

/**
 * \file module.hpp
 * \brief Javascript API module.
 */

/**
 * \defgroup Javascript modules.
 * \brief Modules for the JavaScript API.
 */

#include <cassert>
#include <memory>

#include "sysconfig.hpp"
#include "util.hpp"

namespace irccd {

class irccd;
class js_plugin;

/**
 * \brief JavaScript API module.
 */
class module {
private:
    std::string name_;

public:
    /**
     * Default constructor.
     *
     * \pre !name.empty()
     */
    inline module(std::string name) noexcept
        : name_(std::move(name))
    {
        assert(!name_.empty());
    }

    /**
     * Virtual destructor defaulted.
     */
    virtual ~module() noexcept = default;

    /**
     * Get the module name.
     *
     * \return the name
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }

    /**
     * Load the module into the JavaScript plugin.
     *
     * \param irccd the irccd instance
     * \param plugin the plugin
     */
    virtual void load(irccd& irccd, std::shared_ptr<js_plugin> plugin)
    {
        util::unused(irccd, plugin);
    }
};

} // !irccd

#endif // !IRCCD_MODULE_HPP

/*
 * js_directory_module.hpp -- irccd.Directory API
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

#ifndef IRCCD_JS_DIRECTORY_MODULE_HPP
#define IRCCD_JS_DIRECTORY_MODULE_HPP

/**
 * \file mod-directory.hpp
 * \brief irccd.Directory JavaScript API.
 */

#include "module.hpp"

namespace irccd {

/**
 * \brief irccd.Directory JavaScript API.
 * \ingroup modules
 */
class js_directory_module : public module {
public:
    /**
     * Constructor.
     */
    js_directory_module() noexcept;

    /**
     * \copydoc Module::load
     */
    void load(irccd &irccd, std::shared_ptr<js_plugin> plugin) override;
};

} // !irccd

#endif // !IRCCD_JS_DIRECTORY_MODULE_HPP

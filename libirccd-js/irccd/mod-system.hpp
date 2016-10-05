/*
 * mod-system.hpp -- Irccd.System API
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

#ifndef IRCCD_MOD_SYSTEM_HPP
#define IRCCD_MOD_SYSTEM_HPP

/**
 * \file mod-system.hpp
 * \brief Irccd.System JavaScript API.
 */

#include "module.hpp"

namespace irccd {

/**
 * \brief Irccd.System JavaScript API.
 * \ingroup modules
 */
class SystemModule : public Module {
public:
    /**
     * Irccd.System.
     */
    IRCCD_EXPORT SystemModule() noexcept;

    /**
     * \copydoc Module::load
     */
    IRCCD_EXPORT void load(Irccd &irccd, const std::shared_ptr<JsPlugin> &plugin) override;
};

} // !irccd

#endif // !IRCCD_MOD_SYSTEM_HPP

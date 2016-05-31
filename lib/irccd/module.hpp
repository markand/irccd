/*
 * module.hpp -- JavaScript API module
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

#ifndef IRCCD_MODULE_HPP
#define IRCCD_MODULE_HPP

/**
 * \file module.hpp
 * \brief JavaScript API module.
 */

/**
 * \defgroup modules JavaScript modules
 * \brief Modules for the JavaScript API.
 */

#include <cassert>

#include "sysconfig.hpp"
#include "util.hpp"

namespace irccd {

class Irccd;
class JsPlugin;

/**
 * \brief JavaScript API module.
 */
class Module {
private:
	std::string m_name;

public:
	/**
	 * Default constructor.
	 *
	 * \pre !name.empty()
	 */
	inline Module(std::string name) noexcept
		: m_name(std::move(name))
	{
		assert(!m_name.empty());
	}

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~Module() = default;

	/**
	 * Get the module name.
	 *
	 * \return the name
	 */
	inline const std::string &name() const noexcept
	{
		return m_name;
	}

	/**
	 * Load the module into the JavaScript plugin.
	 *
	 * \param irccd the irccd instance
	 * \param plugin the plugin
	 */
	virtual void load(Irccd &irccd, JsPlugin &plugin)
	{
		util::unused(irccd, plugin);
	}

	/**
	 * Unload the module from the JavaScript plugin.
	 *
	 * \param irccd the irccd instance
	 * \param plugin the plugin
	 */
	virtual void unload(Irccd &irccd, JsPlugin &plugin)
	{
		util::unused(irccd, plugin);
	}
};

} // !irccd

#endif // !IRCCD_MODULE_HPP

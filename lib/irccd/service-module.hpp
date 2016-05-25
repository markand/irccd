/*
 * service-module.hpp -- store and manage JavaScript modules
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

#ifndef IRCCD_SERVICE_MODULE_HPP
#define IRCCD_SERVICE_MODULE_HPP

/**
 * \file service-module.hpp
 * \brief Store and manage JavaScript modules.
 */

#include <memory>
#include <vector>

#include "sysconfig.hpp"

namespace irccd {

class Module;

/**
 * \brief Store and manage JavaScript modules.
 */
class ModuleService {
private:
	std::vector<std::shared_ptr<Module>> m_modules;

public:
	/**
	 * Construct the service and predefined irccd API.
	 */
	IRCCD_EXPORT ModuleService();

	/**
	 * Get all modules.
	 *
	 * \return the modules
	 */
	inline const std::vector<std::shared_ptr<Module>> &modules() const noexcept
	{
		return m_modules;
	}

	/**
	 * Get a module.
	 *
	 * \param name the module name
	 * \return the module or empty if not found
	 */
	IRCCD_EXPORT std::shared_ptr<Module> get(const std::string &name) const noexcept;

	/**
	 * Tells if a module exist.
	 *
	 * \param name the name
	 */
	IRCCD_EXPORT bool contains(const std::string &name) const;

	/**
	 * Add a JavaScript API module.
	 *
	 * \pre module != nullptr
	 * \pre !contains(module)
	 * \param module the module
	 */
	IRCCD_EXPORT void add(std::shared_ptr<Module> module);
};

} // !irccd

#endif // !IRCCD_SERVICE_MODULE_HPP

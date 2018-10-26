/*
 * dynlib_plugin.hpp -- native plugin implementation
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

#ifndef IRCCD_DAEMON_DYNLIB_PLUGIN_HPP
#define IRCCD_DAEMON_DYNLIB_PLUGIN_HPP

/**
 * \file dynlib_plugin.hpp
 * \brief Native plugin implementation.
 */

#include "plugin.hpp"

namespace irccd {

/**
 * \brief Implementation for searching native plugins.
 */
class dynlib_plugin_loader : public plugin_loader {
public:
	/**
	 * Constructor.
	 *
	 * \param directories optional directories to search, if empty use defaults.
	 */
	dynlib_plugin_loader(std::vector<std::string> directories = {}) noexcept;

	/**
	 * \copydoc plugin_loader::open
	 */
	auto open(std::string_view id, std::string_view file) -> std::shared_ptr<plugin> override;
};

} // !irccd

#endif // !IRCCD_DAEMON_DYNLIB_PLUGIN_HPP

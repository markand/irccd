/*
 * mod-irccd.hpp -- Irccd API
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

#ifndef IRCCD_MOD_IRCCD_HPP
#define IRCCD_MOD_IRCCD_HPP

/**
 * \file mod-irccd.hpp
 * \brief Irccd JavaScript API.
 */

#include <cerrno>
#include <cstring>
#include <string>

#include "js.hpp"
#include "module.hpp"

namespace irccd {

class Irccd;

/**
 * \brief Custom JavaScript exception for system error.
 */
class SystemError {
private:
	int m_errno;
	std::string m_message;

public:
	/**
	 * Create a system error from the current errno value.
	 */
	SystemError();

	/**
	 * Create a system error with the given errno and message.
	 *
	 * \param e the errno number
	 * \param message the message
	 */
	SystemError(int e, std::string message);

	/**
	 * Raise the SystemError.
	 *
	 * \param ctx the context
	 */
	void raise(duk::ContextPtr ctx) const;
};

/**
 * Irccd JavaScript API.
 */
class IrccdModule : public Module {
public:
	/**
	 * Irccd.
	 */
	IrccdModule() noexcept;

	/**
	 * \copydoc Module::load
	 */
	void load(Irccd &irccd, JsPlugin &plugin) override;
};

} // !irccd

#endif // !IRCCD_MOD_IRCCD_HPP
/*
 * scope_exit.hpp -- do something on scope exit
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_SCOPE_EXIT_HPP
#define IRCCD_SCOPE_EXIT_HPP

/**
 * \file scope_exit.hpp
 * \brief Do something on scope exit.
 */

#include <functional>

namespace irccd {

/**
 * \brief Do something on scope exit.
 */
class scope_exit {
private:
	std::function<void ()> func_;

public:
	/**
	 * Constructor.
	 *
	 * \pre func != nullptr
	 * \param func the function to call
	 */
	scope_exit(std::function<void ()> func) noexcept;

	/**
	 * Execute the handler.
	 */
	~scope_exit() noexcept;
};

} // !irccd

#endif // !SCOPE_EXIT_HPP

/*
 * mock.hpp -- keep track of function invocations
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

#ifndef IRCCD_TEST_MOCK_HPP
#define IRCCD_TEST_MOCK_HPP

/**
 * \file mock.hpp
 * \brief Keep track of function invocations.
 */

#include <any>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

namespace irccd::test {

/**
 * \brief Keep track of function invocations.
 */
class mock {
public:
	/**
	 * \brief Functions arguments.
	 */
	using args = std::vector<std::any>;

	/**
	 * \brief Map of all functions.
	 */
	using functions = std::unordered_map<std::string, std::vector<args>>;

private:
	mutable functions table_;

public:
	/**
	 * Register a new function invocation.
	 *
	 * \param name the function name
	 * \param args the arguments list
	 */
	void push(std::string name, args args = {}) const;

	/**
	 * Get all function invocations by name.
	 *
	 * \param name the function name
	 * \return the list of functions and their arguments or empty if not called
	 */
	auto find(const std::string& name) const -> std::vector<args>;

	/**
	 * Clear all function invocations by name.
	 *
	 * \param name the function name
	 */
	void clear(const std::string& name) const noexcept;

	/**
	 * Clear all function invocations.
	 */
	void clear() const noexcept;

	/**
	 * Tells if no functions have been called.
	 *
	 * \return true if no functions have been called
	 */
	auto empty() const noexcept -> bool;
};

} // !irccd::test

#endif // !IRCCD_TEST_MOCK_HPP

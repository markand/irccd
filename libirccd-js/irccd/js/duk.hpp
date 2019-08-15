/*
 * duk.hpp -- miscellaneous Duktape extras
 *
 * Copyright (c) 2017-2019 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_JS_DUK_HPP
#define IRCCD_JS_DUK_HPP

/**
 * \file duk.hpp
 * \brief Miscellaneous Duktape extras
 * \author David Demelier <markand@malikania.fr>
 * \version 0.2.0
 */

#include <exception>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "duktape.h"

namespace irccd {

namespace js {

/**
 * \brief Miscellaneous Duktape extras.
 */
namespace duk {

// {{{ stack_guard

/**
 * \brief Stack sanity checker.
 *
 * Instanciate this class where you need to manipulate the Duktape stack outside
 * a Duktape/C function, its destructor will examinate if the stack size matches
 * the user expected size.
 *
 * When compiled with NDEBUG, this class does nothing.
 *
 * To use it, just declare an lvalue at the beginning of your function.
 */
class stack_guard {
#if !defined(NDEBUG)
private:
	duk_context* context_;
	unsigned expected_;
	int at_start_;
#endif

public:
	/**
	 * Create the stack checker.
	 *
	 * No-op if NDEBUG is set.
	 *
	 * \param ctx the context
	 * \param expected the size expected relative to the already existing values
	 */
	stack_guard(duk_context* ctx, unsigned expected = 0) noexcept;

	/**
	 * Verify the expected size.
	 *
	 * No-op if NDEBUG is set.
	 */
	~stack_guard() noexcept;
};

// }}}

// {{{ context

/**
 * \brief RAII based Duktape handler.
 *
 * This class is implicitly convertible to duk_context for convenience.
 */
class context {
private:
	std::unique_ptr<duk_context, void (*)(duk_context*)> handle_;

	context(const context&) = delete;
	void operator=(const context&) = delete;

public:
	/**
	 * Create default context.
	 */
	context() noexcept;

	/**
	 * Default move constructor.
	 */
	context(context&&) noexcept = default;

	/**
	 * Convert the context to the native Duktape/C type.
	 *
	 * \return the duk_context
	 */
	operator duk_context*() noexcept;

	/**
	 * Convert the context to the native Duktape/C type.
	 *
	 * \return the duk_context
	 */
	operator duk_context*() const noexcept;

	/**
	 * Default move assignment operator.
	 *
	 * \return this
	 */
	auto operator=(context&&) noexcept -> context& = default;
};

// }}}

// {{{ stack_info

/**
 * \brief Error description.
 *
 * This class fills the fields got in an Error object.
 */
class stack_info : public std::exception {
private:
	std::string name_;
	std::string message_;
	std::string stack_;
	std::string file_name_;
	unsigned line_number_;

public:
	/**
	 * Construct the stack information.
	 *
	 * \param name the exception name (e.g. ReferenceError)
	 * \param message the error message
	 * \param stack the stack trace
	 * \param file_name the optional filename
	 * \param line_number the optional line number
	 */
	stack_info(std::string name,
	           std::string message,
	           std::string stack,
	           std::string file_name,
	           unsigned line_number = 0) noexcept;

	/**
	 * Get the exception name.
	 *
	 * \return the exception name (e.g. ReferenceError)
	 */
	auto get_name() const noexcept -> const std::string&;

	/**
	 * Get the error message.
	 *
	 * \return the message
	 */
	auto get_message() const noexcept -> const std::string&;

	/**
	 * Get the stack trace.
	 *
	 * \return the stack
	 */
	auto get_stack() const noexcept -> const std::string&;

	/**
	 * Get the optional file name.
	 *
	 * \return the file name
	 */
	auto get_file_name() const noexcept -> const std::string&;

	/**
	 * Get the line number.
	 *
	 * \return the line number
	 */
	auto get_line_number() const noexcept -> unsigned;

	/**
	 * Get the error message. This effectively returns message field.
	 *
	 * \return the message
	 */
	auto what() const noexcept -> const char* override;
};

// }}}

// {{{ type_traits

/**
 * \brief Operations on different types.
 *
 * This class provides some functions for the given type, depending on the
 * nature of the function.
 *
 * For example, push will call type_traits<T>::push static function
 * if the type_traits is implemented for that given T type.
 *
 * This helps passing/getting function between Javascript and C++ code.
 *
 * Example:
 *
 * ```cpp
 * push(ctx, 123);	 // Uses type_traits<int>
 * push(ctx, true);	// Uses type_traits<bool>
 * ```
 *
 * This class is specialized for the following types:
 *
 *   - `bool`,
 *   - `duk_int_t`,
 *   - `duk_uint_t`,
 *   - `duk_double_t`,
 *   - `const char*`,
 *   - `std::string`,
 *   - `std::string_view`.
 *
 * Regarding exceptions, this class is specialized for the following types:
 *
 *   - error,
 *   - std::exception,
 *
 * \see push
 * \see get
 * \see require
 * \see raise
 */
template <typename T>
struct type_traits;

// }}}

// {{{ push

/**
 * Generic push function.
 *
 * This function calls type_traits<T>::push if specialized.
 *
 * \param ctx the Duktape context
 * \param value the forwarded value
 * \return 1 for convenience
 */
template <typename T>
auto push(duk_context* ctx, T&& value) -> int
{
	using Type = typename std::decay<T>::type;

	type_traits<Type>::push(ctx, std::forward<T>(value));

	return 1;
}

// }}}

// {{{ get

/**
 * Generic get function.
 *
 * This functions calls type_traits<T>::get if specialized.
 *
 * \param ctx the Duktape context
 * \param index the value index
 * \return the converted value
 */
template <typename T>
auto get(duk_context* ctx, duk_idx_t index)
{
	using Type = typename std::decay<T>::type;

	return type_traits<Type>::get(ctx, index);
}

// }}}

// {{{ require

/**
 * Generic require function.
 *
 * This functions calls type_traits<T>::require if specialized.
 *
 * \param ctx the Duktape context
 * \param index the value index
 * \return the converted value
 */
template <typename T>
auto require(duk_context* ctx, duk_idx_t index)
{
	using Type = typename std::decay<T>::type;

	return type_traits<Type>::require(ctx, index);
}

// }}}

// {{{ error

/**
 * \brief Base ECMAScript error class.
 * \warning Override the function create for your own exceptions
 */
class error {
private:
	int type_{DUK_ERR_ERROR};
	std::string message_;

protected:
	/**
	 * Constructor with a type of error specified, specially designed for
	 * derived errors.
	 *
	 * \param type of error (e.g. DUK_ERR_ERROR)
	 * \param message the message
	 */
	error(int type, std::string message) noexcept;

public:
	/**
	 * Constructor with a message.
	 *
	 * \param message the message
	 */
	error(std::string message) noexcept;

	/**
	 * Virtual destructor defaulted.
	 */
	~error() = default;

	/**
	 * Create the exception on the stack.
	 *
	 * \note the default implementation search for the global variables
	 * \param ctx the context
	 */
	void create(duk_context* ctx) const;
};

// }}}

// {{{ eval_error

/**
 * \brief Error in eval() function.
 */
class eval_error : public error {
public:
	/**
	 * Construct an EvalError.
	 *
	 * \param message the message
	 */
	eval_error(std::string message) noexcept;
};

// }}}

// {{{ range_error

/**
 * \brief Value is out of range.
 */
class range_error : public error {
public:
	/**
	 * Construct an RangeError.
	 *
	 * \param message the message
	 */
	range_error(std::string message) noexcept;
};

// }}}

// {{{ reference_error

/**
 * \brief Trying to use a variable that does not exist.
 */
class reference_error : public error {
public:
	/**
	 * Construct an ReferenceError.
	 *
	 * \param message the message
	 */
	reference_error(std::string message) noexcept;
};

// }}}

// {{{ syntax_error

/**
 * \brief Syntax error in the script.
 */
class syntax_error : public error {
public:
	/**
	 * Construct an SyntaxError.
	 *
	 * \param message the message
	 */
	syntax_error(std::string message) noexcept;
};

// }}}

// {{{ type_error

/**
 * \brief Invalid type given.
 */
class type_error : public error {
public:
	/**
	 * Construct an TypeError.
	 *
	 * \param message the message
	 */
	type_error(std::string message) noexcept;
};

// }}}

// {{{ uri_error

/**
 * \brief URI manipulation failure.
 */
class uri_error : public error {
public:
	/**
	 * Construct an URIError.
	 *
	 * \param message the message
	 */
	uri_error(std::string message) noexcept;
};

// }}}

// {{{ raise

/**
 * Create an exception into the stack and throws it.
 *
 * This function needs the following requirements in type_traits
 *
 * ```cpp
 * static void raise(duk_context*, Error);
 * ```
 *
 * Error can be any kind of value, it is forwarded.
 *
 * \param ctx the Duktape context
 * \param error the error object
 */
template <typename Error>
void raise(duk_context* ctx, Error&& error)
{
	using type = std::decay_t<Error>;

	type_traits<type>::raise(ctx, std::forward<Error>(error));
}

// }}}

// {{{ get_stack

/**
 * Get the error object when a JavaScript error has been thrown (e.g. eval
 * failure).
 *
 * \param ctx the context
 * \param index the index
 * \param pop if true, also remove the exception from the stack
 * \return the information
 */
auto get_stack(duk_context* ctx, int index, bool pop = true) -> stack_info;

// }}}

// {{{ type_traits<std::exception>

/**
 * \brief Specialization for std::exception.
 */
template <>
struct type_traits<std::exception> {
	/**
	 * Raise a Error object.
	 *
	 * \param ctx the Duktape context
	 * \param ex the exception
	 */
	static void raise(duk_context* ctx, const std::exception& ex);
};

// }}}

// {{{ type_traits<error>

/**
 * \brief Specialization for error.
 */
template <>
struct type_traits<error> {
	/**
	 * Raise a error.
	 *
	 * \param ctx the Duktape context
	 * \param ex the exception
	 */
	static void raise(duk_context* ctx, const error& ex);
};

// }}}

// {{{ type_traits<bool>

/**
 * \brief Specialization for bool.
 */
template <>
struct type_traits<bool> {
	/**
	 * Push a boolean.
	 *
	 * Uses duk_push_boolean
	 *
	 * \param ctx the Duktape context
	 * \param value the value
	 */
	static void push(duk_context* ctx, bool value);

	/**
	 * Get a boolean.
	 *
	 * Uses duk_get_boolean.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto get(duk_context* ctx, duk_idx_t index) -> bool;

	/**
	 * Require a boolean.
	 *
	 * Uses duk_require_boolean.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> bool;
};

// }}}

// {{{ type_traits<duk_double_t>

/**
 * \brief Specialization for duk_double_t.
 */
template <>
struct type_traits<duk_double_t> {
	/**
	 * Push a double.
	 *
	 * Uses duk_push_number
	 *
	 * \param ctx the Duktape context
	 * \param value the value
	 */
	static void push(duk_context* ctx, duk_double_t value);

	/**
	 * Get a double.
	 *
	 * Uses duk_get_number.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto get(duk_context* ctx, duk_idx_t index) -> duk_double_t;

	/**
	 * Require a double.
	 *
	 * Uses duk_require_double.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> duk_double_t;
};

// }}}

// {{{ type_traits<duk_int_t>

/**
 * \brief Specialization for duk_int_t.
 */
template <>
struct type_traits<duk_int_t> {
	/**
	 * Push an int.
	 *
	 * Uses duk_push_int
	 *
	 * \param ctx the Duktape context
	 * \param value the value
	 */
	static void push(duk_context* ctx, duk_int_t value);

	/**
	 * Get an int.
	 *
	 * Uses duk_get_number.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto get(duk_context* ctx, duk_idx_t index) -> duk_int_t;

	/**
	 * Require an int.
	 *
	 * Uses duk_require_int.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> duk_int_t;
};

// }}}

// {{{ type_traits<duk_uint_t>

/**
 * \brief Specialization for duk_uint_t.
 */
template <>
struct type_traits<duk_uint_t> {
	/**
	 * Push an unsigned int.
	 *
	 * Uses duk_push_uint
	 *
	 * \param ctx the Duktape context
	 * \param value the value
	 */
	static void push(duk_context* ctx, duk_uint_t value);

	/**
	 * Get an unsigned int.
	 *
	 * Uses duk_get_uint.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto get(duk_context* ctx, duk_idx_t index) -> duk_uint_t;

	/**
	 * Require an unsigned int.
	 *
	 * Uses duk_require_uint.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> duk_uint_t;
};

// }}}

// {{{ type_traits<const char*>

/**
 * \brief Specialization for C strings.
 */
template <>
struct type_traits<const char*> {
	/**
	 * Push a C string.
	 *
	 * Uses duk_push_string
	 *
	 * \param ctx the Duktape context
	 * \param value the value
	 */
	static void push(duk_context* ctx, const char* value);

	/**
	 * Get a C string.
	 *
	 * Uses duk_get_string.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto get(duk_context* ctx, duk_idx_t index) -> const char*;

	/**
	 * Require a C string.
	 *
	 * Uses duk_require_string.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> const char*;
};

// }}}

// {{{ type_traits<std::string>

/**
 * \brief Specialization for C++ std::strings.
 */
template <>
struct type_traits<std::string> {
	/**
	 * Push a C++ std::string.
	 *
	 * Uses duk_push_lstring
	 *
	 * \param ctx the Duktape context
	 * \param value the value
	 */
	static void push(duk_context* ctx, const std::string& value);

	/**
	 * Get a C++ std::string.
	 *
	 * Uses duk_get_lstring.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto get(duk_context* ctx, duk_idx_t index) -> std::string;

	/**
	 * Require a C++ std::string.
	 *
	 * Uses duk_require_lstring.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> std::string;
};

// }}}

// {{{ type_traits<std::string_view>

/**
 * \brief Specialization for C++ std::string_views.
 */
template <>
struct type_traits<std::string_view> : public std::true_type {
	/**
	 * Push a C++ std::string_view.
	 *
	 * Uses duk_push_lstring
	 *
	 * \param ctx the Duktape context
	 * \param value the value
	 */
	static void push(duk_context* ctx, std::string_view value);

	/**
	 * Get a C++ std::string_view.
	 *
	 * Uses duk_get_lstring.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto get(duk_context* ctx, duk_idx_t index) -> std::string_view;

	/**
	 * Require a C++ std::string_view.
	 *
	 * Uses duk_require_lstring.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> std::string_view;
};

// }}}

// {{{ type_traits<std::vector<T>>

/**
 * \brief Specialization for std::vector<T>
 */
template <typename T>
struct type_traits<std::vector<T>> : public std::true_type {
	/**
	 * Push a C++ std::vector<T>.
	 *
	 * \param ctx the Duktape context
	 * \param values the array
	 */
	static void push(duk_context* ctx, const std::vector<T>& values)
	{
		duk_push_array(ctx);

		for (std::size_t i = 0; i < values.size(); ++i) {
			type_traits<T>::push(ctx, values[i]);
			duk_put_prop_index(ctx, -2, i);
		}
	}

	/**
	 * Get a C++ std::vector<T>.
	 *
	 * \param ctx the Duktape context
	 * \param index the value index
	 * \return the converted value
	 */
	static auto get(duk_context* ctx, duk_idx_t index) -> std::vector<T>
	{
		const auto length = duk_get_length(ctx, index);

		std::vector<T> result;

		result.reserve(length);

		for (auto i = 0U; i < length; ++i) {
			duk_get_prop_index(ctx, index, i);
			result.push_back(type_traits<T>::get(ctx, -1));
			duk_pop(ctx);
		}

		return result;
	}
};

// }}}

} // !duk

} // !js

} // !irccd

#endif // !IRCCD_JS_DUK_HPP

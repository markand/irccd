/*
 * duktape.hpp -- miscellaneous Duktape extras
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

#ifndef IRCCD_JS_DUKTAPE_HPP
#define IRCCD_JS_DUKTAPE_HPP

/**
 * \file duktape.hpp
 * \brief Miscellaneous Duktape extras
 * \author David Demelier <markand@malikania.fr>
 * \version 0.2.0
 */

#include <cassert>
#include <cstdio>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "duktape.h"

namespace irccd {

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
class dukx_stack_assert {
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
    inline dukx_stack_assert(duk_context* ctx, unsigned expected = 0) noexcept
#if !defined(NDEBUG)
        : context_(ctx)
        , expected_(expected)
        , at_start_(duk_get_top(ctx))
#endif
    {
#if defined(NDEBUG)
        (void)ctx;
        (void)expected;
#endif
    }

    /**
     * Verify the expected size.
     *
     * No-op if NDEBUG is set.
     */
    inline ~dukx_stack_assert() noexcept
    {
#if !defined(NDEBUG)
        auto result = duk_get_top(context_) - at_start_;

        if (result != static_cast<int>(expected_)) {
            std::fprintf(stderr, "Corrupt stack detection in dukx_stack_assert:\n");
            std::fprintf(stderr, "  Size at start:           %d\n", at_start_);
            std::fprintf(stderr, "  Size at end:             %d\n", duk_get_top(context_));
            std::fprintf(stderr, "  Expected (user):         %u\n", expected_);
            std::fprintf(stderr, "  Expected (adjusted):     %u\n", expected_ + at_start_);
            std::fprintf(stderr, "  Difference count:       %+d\n", result - expected_);
            std::abort();
        }
#endif
    }
};

/**
 * \brief RAII based Duktape handler.
 *
 * This class is implicitly convertible to duk_context for convenience.
 */
class dukx_context {
private:
    std::unique_ptr<duk_context, void (*)(duk_context*)> handle_;

    dukx_context(const dukx_context&) = delete;
    dukx_context &operator=(const dukx_context&) = delete;

public:
    /**
     * Create default context.
     */
    inline dukx_context() noexcept
        : handle_(duk_create_heap_default(), duk_destroy_heap)
    {
    }

    /**
     * Default move constructor.
     */
    dukx_context(dukx_context&&) noexcept = default;

    /**
     * Convert the context to the native Duktape/C type.
     *
     * \return the duk_context
     */
    inline operator duk_context*() noexcept
    {
        return handle_.get();
    }

    /**
     * Convert the context to the native Duktape/C type.
     *
     * \return the duk_context
     */
    inline operator duk_context*() const noexcept
    {
        return handle_.get();
    }

    /**
     * Default move assignment operator.
     *
     * \return this
     */
    dukx_context& operator=(dukx_context&&) noexcept = delete;
};

/**
 * \brief Error description.
 *
 * This class fills the fields got in an Error object.
 */
class dukx_stack_info : public std::exception {
private:
    std::string name_;
    std::string message_;
    std::string stack_;
    std::string file_name_;
    int line_number_;

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
    inline dukx_stack_info(std::string name,
                           std::string message,
                           std::string stack,
                           std::string file_name,
                           int line_number = 0) noexcept
        : name_(std::move(name))
        , message_(std::move(message))
        , stack_(std::move(stack))
        , file_name_(std::move(file_name))
        , line_number_(line_number)
    {
    }

    /**
     * Get the exception name.
     *
     * \return the exception name (e.g. ReferenceError)
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }

    /**
     * Get the error message.
     *
     * \return the message
     */
    inline const std::string& message() const noexcept
    {
        return message_;
    }

    /**
     * Get the stack trace.
     *
     * \return the stack
     */
    inline const std::string& stack() const noexcept
    {
        return stack_;
    }

    /**
     * Get the optional file name.
     *
     * \return the file name
     */
    inline const std::string& file_name() const noexcept
    {
        return file_name_;
    }

    /**
     * Get the line number.
     *
     * \return the line number
     */
    inline int line_number() const noexcept
    {
        return line_number_;
    }

    /**
     * Get the error message. This effectively returns message field.
     *
     * \return the message
     */
    const char* what() const noexcept override
    {
        return message_.c_str();
    }
};

/**
 * \brief Operations on different types.
 *
 * This class provides some functions for the given type, depending on the
 * nature of the function.
 *
 * For example, dukx_push will call dukx_type_traits<T>::push static function
 * if the dukx_type_traits is implemented for that given T type.
 *
 * This helps passing/getting function between Javascript and C++ code.
 *
 * Example:
 *
 * ```cpp
 * dukx_push(ctx, 123);     // Uses dukx_type_traits<int>
 * dukx_push(ctx, true);    // Uses dukx_type_traits<bool>
 * ```
 *
 * This class is specialized for the following types:
 *
 *   - `bool`,
 *   - `duk_int_t`,
 *   - `duk_uint_t`,
 *   - `duk_double_t`,
 *   - `const char*`,
 *   - `std::string`
 */
template <typename T>
class dukx_type_traits : public std::false_type {
};

/**
 * \brief Specialization for bool.
 */
template <>
class dukx_type_traits<bool> : public std::true_type {
public:
    /**
     * Push a boolean.
     *
     * Uses duk_push_boolean
     *
     * \param ctx the Duktape context
     * \param value the value
     */
    static void push(duk_context* ctx, bool value)
    {
        duk_push_boolean(ctx, value);
    }

    /**
     * Get a boolean.
     *
     * Uses duk_get_boolean.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static bool get(duk_context* ctx, duk_idx_t index)
    {
        return duk_get_boolean(ctx, index);
    }

    /**
     * Require a boolean.
     *
     * Uses duk_require_boolean.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static bool require(duk_context* ctx, duk_idx_t index)
    {
        return duk_require_boolean(ctx, index);
    }
};

/**
 * \brief Specialization for duk_double_t.
 */
template <>
class dukx_type_traits<duk_double_t> : public std::true_type {
public:
    /**
     * Push a double.
     *
     * Uses duk_push_number
     *
     * \param ctx the Duktape context
     * \param value the value
     */
    static void push(duk_context* ctx, duk_double_t value)
    {
        duk_push_number(ctx, value);
    }

    /**
     * Get a double.
     *
     * Uses duk_get_number.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static duk_double_t get(duk_context* ctx, duk_idx_t index)
    {
        return duk_get_number(ctx, index);
    }

    /**
     * Require a double.
     *
     * Uses duk_require_double.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static duk_double_t require(duk_context* ctx, duk_idx_t index)
    {
        return duk_require_number(ctx, index);
    }
};

/**
 * \brief Specialization for duk_int_t.
 */
template <>
class dukx_type_traits<duk_int_t> : public std::true_type {
public:
    /**
     * Push an int.
     *
     * Uses duk_push_int
     *
     * \param ctx the Duktape context
     * \param value the value
     */
    static void push(duk_context* ctx, duk_int_t value)
    {
        duk_push_int(ctx, value);
    }

    /**
     * Get an int.
     *
     * Uses duk_get_number.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static duk_int_t get(duk_context* ctx, duk_idx_t index)
    {
        return duk_get_int(ctx, index);
    }

    /**
     * Require an int.
     *
     * Uses duk_require_int.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static duk_int_t require(duk_context* ctx, duk_idx_t index)
    {
        return duk_require_int(ctx, index);
    }
};

/**
 * \brief Specialization for duk_uint_t.
 */
template <>
class dukx_type_traits<duk_uint_t> : public std::true_type {
public:
    /**
     * Push an unsigned int.
     *
     * Uses duk_push_uint
     *
     * \param ctx the Duktape context
     * \param value the value
     */
    static void push(duk_context* ctx, duk_uint_t value)
    {
        duk_push_uint(ctx, value);
    }

    /**
     * Get an unsigned int.
     *
     * Uses duk_get_uint.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static duk_uint_t get(duk_context* ctx, duk_idx_t index)
    {
        return duk_get_uint(ctx, index);
    }

    /**
     * Require an unsigned int.
     *
     * Uses duk_require_uint.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static duk_uint_t require(duk_context* ctx, duk_idx_t index)
    {
        return duk_require_uint(ctx, index);
    }
};

/**
 * \brief Specialization for C strings.
 */
template <>
class dukx_type_traits<const char*> : public std::true_type {
public:
    /**
     * Push a C string.
     *
     * Uses duk_push_string
     *
     * \param ctx the Duktape context
     * \param value the value
     */
    static void push(duk_context* ctx, const char* value)
    {
        duk_push_string(ctx, value);
    }

    /**
     * Get a C string.
     *
     * Uses duk_get_string.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static const char* get(duk_context* ctx, duk_idx_t index)
    {
        return duk_get_string(ctx, index);
    }

    /**
     * Require a C string.
     *
     * Uses duk_require_string.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static const char* require(duk_context* ctx, duk_idx_t index)
    {
        return duk_require_string(ctx, index);
    }
};

/**
 * \brief Specialization for C++ std::strings.
 */
template <>
class dukx_type_traits<std::string> : public std::true_type {
public:
    /**
     * Push a C++ std::string.
     *
     * Uses duk_push_lstring
     *
     * \param ctx the Duktape context
     * \param value the value
     */
    static void push(duk_context* ctx, const std::string& value)
    {
        duk_push_lstring(ctx, value.data(), value.size());
    }

    /**
     * Get a C++ std::string.
     *
     * Uses duk_get_lstring.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static std::string get(duk_context* ctx, duk_idx_t index)
    {
        duk_size_t length;
        const char* str = duk_get_lstring(ctx, index, &length);

        return {str, length};
    }

    /**
     * Require a C++ std::string.
     *
     * Uses duk_require_lstring.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the converted value
     */
    static std::string require(duk_context* ctx, duk_idx_t index)
    {
        duk_size_t length;
        const char* str = duk_require_lstring(ctx, index, &length);

        return {str, length};
    }
};

/**
 * \brief Partial specialization for collections.
 *
 * Derive from this class to implement type traits for collections.
 *
 * \see duktape_vector.hpp
 */
template <typename Container>
class dukx_array_type_traits : public std::true_type {
public:
    /**
     * Push a Javascript array by copying values.
     *
     * Uses dukx_push for each T values in the collection.
     *
     * \param ctx the Duktape context
     * \param c the container
     */
    static void push(duk_context* ctx, const Container& c)
    {
        using Type = typename Container::value_type;

        duk_push_array(ctx);

        int i = 0;

        for (auto v : c) {
            dukx_type_traits<Type>::push(ctx, v);
            duk_put_prop_index(ctx, -2, i++);
        }
    }

    /**
     * Get an array from the Javascript array.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the container
     */
    static Container get(duk_context* ctx, duk_idx_t index)
    {
        using Type = typename Container::value_type;
        using Size = typename Container::size_type;

        Container result;
        Size length = duk_get_length(ctx, index);

        for (Size i = 0; i < length; ++i) {
            duk_get_prop_index(ctx, index, i);
            result.push_back(dukx_type_traits<Type>::get(ctx, -1));
            duk_pop(ctx);
        }

        return result;
    }

    /**
     * Require an array from the Javascript array.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the container
     */
    static Container require(duk_context* ctx, duk_idx_t index)
    {
        duk_check_type(ctx, index, DUK_TYPE_OBJECT);

        return get(ctx, index);
    }
};

/**
 * \brief Partial specialization for maps.
 *
 * Derive from this class to implement type traits for maps.
 *
 * \see duktape_vector.hpp
 */
template <typename Container>
class dukx_object_type_traits : public std::true_type {
public:
    /**
     * Push a container by copying values.
     *
     * Uses dukx_push for each key/value pair in the container.
     *
     * \param ctx the Duktape context
     * \param c the container
     */
    static void push(duk_context* ctx, const Container& c)
    {
        using Type = typename Container::mapped_type;

        duk_push_object(ctx);

        for (const auto& pair : c) {
            dukx_type_traits<std::string>::push(ctx, pair.first);
            dukx_type_traits<Type>::push(ctx, pair.second);
            duk_put_prop(ctx, -3);
        }
    }

    /**
     * Get a object from the Javascript array.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the container
     */
    static Container get(duk_context* ctx, duk_idx_t index)
    {
        using Type = typename Container::mapped_type;

        Container result;

        duk_enum(ctx, index, 0);

        while (duk_next(ctx, -1, true)) {
            result.emplace(
                dukx_type_traits<std::string>::get(ctx, -2),
                dukx_type_traits<Type>::get(ctx, -1)
            );
            duk_pop_n(ctx, 2);
        }

        duk_pop(ctx);

        return result;
    }

    /**
     * Require a object from the Javascript array.
     *
     * \param ctx the Duktape context
     * \param index the value index
     * \return the container
     */
    static Container require(duk_context* ctx, duk_idx_t index)
    {
        duk_check_type(ctx, index, DUK_TYPE_OBJECT);

        return get(ctx, index);
    }
};

/**
 * Generic push function.
 *
 * This function calls dukx_type_traits<T>::push if specialized.
 *
 * \param ctx the Duktape context
 * \param value the forwarded value
 * \return 1 for convenience
 */
template <typename T>
int dukx_push(duk_context* ctx, T&& value)
{
    using Type = typename std::decay<T>::type;

    static_assert(dukx_type_traits<Type>::value, "type T not supported");

    dukx_type_traits<Type>::push(ctx, std::forward<T>(value));

    return 1;
}

/**
 * Generic get function.
 *
 * This functions calls dukx_type_traits<T>::get if specialized.
 *
 * \param ctx the Duktape context
 * \param index the value index
 * \return the converted value
 */
template <typename T>
T dukx_get(duk_context* ctx, duk_idx_t index)
{
    using Type = typename std::decay<T>::type;

    static_assert(dukx_type_traits<Type>::value, "type T not supported");

    return dukx_type_traits<Type>::get(ctx, index);
}

/**
 * Generic require function.
 *
 * This functions calls dukx_type_traits<T>::require if specialized.
 *
 * \param ctx the Duktape context
 * \param index the value index
 * \return the converted value
 */
template <typename T>
T dukx_require(duk_context* ctx, duk_idx_t index)
{
    using Type = typename std::decay<T>::type;

    static_assert(dukx_type_traits<Type>::value, "type T not supported");

    return dukx_type_traits<Type>::require(ctx, index);
}

/**
 * \brief Base ECMAScript error class.
 * \warning Override the function create for your own exceptions
 */
class dukx_error {
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
    inline dukx_error(int type, std::string message) noexcept
        : type_(type)
        , message_(std::move(message))
    {
    }

public:
    /**
     * Constructor with a message.
     *
     * \param message the message
     */
    inline dukx_error(std::string message) noexcept
        : message_(std::move(message))
    {
    }

    /**
     * Virtual destructor defaulted.
     */
    virtual ~dukx_error() = default;

    /**
     * Create the exception on the stack.
     *
     * \note the default implementation search for the global variables
     * \param ctx the context
     */
    void create(duk_context* ctx) const
    {
        duk_push_error_object(ctx, type_, "%s", message_.c_str());
    }
};

/**
 * \brief Error in eval() function.
 */
class dukx_eval_error : public dukx_error {
public:
    /**
     * Construct an EvalError.
     *
     * \param message the message
     */
    inline dukx_eval_error(std::string message) noexcept
        : dukx_error(DUK_ERR_EVAL_ERROR, std::move(message))
    {
    }
};

/**
 * \brief Value is out of range.
 */
class dukx_range_error : public dukx_error {
public:
    /**
     * Construct an RangeError.
     *
     * \param message the message
     */
    inline dukx_range_error(std::string message) noexcept
        : dukx_error(DUK_ERR_RANGE_ERROR, std::move(message))
    {
    }
};

/**
 * \brief Trying to use a variable that does not exist.
 */
class dukx_reference_error : public dukx_error {
public:
    /**
     * Construct an ReferenceError.
     *
     * \param message the message
     */
    inline dukx_reference_error(std::string message) noexcept
        : dukx_error(DUK_ERR_REFERENCE_ERROR, std::move(message))
    {
    }
};

/**
 * \brief Syntax error in the script.
 */
class dukx_syntax_error : public dukx_error {
public:
    /**
     * Construct an SyntaxError.
     *
     * \param message the message
     */
    inline dukx_syntax_error(std::string message) noexcept
        : dukx_error(DUK_ERR_SYNTAX_ERROR, std::move(message))
    {
    }
};

/**
 * \brief Invalid type given.
 */
class dukx_type_error : public dukx_error {
public:
    /**
     * Construct an TypeError.
     *
     * \param message the message
     */
    inline dukx_type_error(std::string message) noexcept
        : dukx_error(DUK_ERR_TYPE_ERROR, std::move(message))
    {
    }
};

/**
 * \brief URI manipulation failure.
 */
class dukx_uri_error : public dukx_error {
public:
    /**
     * Construct an URIError.
     *
     * \param message the message
     */
    inline dukx_uri_error(std::string message) noexcept
        : dukx_error(DUK_ERR_URI_ERROR, std::move(message))
    {
    }
};

/**
 * Create an exception into the stack and throws it.
 *
 * \param ctx the Duktape context
 * \param error the error object
 */
template <typename Error>
void dukx_throw(duk_context* ctx, const Error& error)
{
    error.create(ctx);

    (void)duk_throw(ctx);
}

/**
 * Get the error object when a JavaScript error has been thrown (e.g. eval
 * failure).
 *
 * \param ctx the context
 * \param index the index
 * \param pop if true, also remove the exception from the stack
 * \return the information
 */
inline dukx_stack_info dukx_stack(duk_context* ctx, int index, bool pop = true)
{
    index = duk_normalize_index(ctx, index);

    duk_get_prop_string(ctx, index, "name");
    auto name = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "message");
    auto message = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "fileName");
    auto file_name = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "lineNumber");
    auto line_number = duk_to_int(ctx, -1);
    duk_get_prop_string(ctx, index, "stack");
    auto stack = duk_to_string(ctx, -1);
    duk_pop_n(ctx, 5);

    if (pop)
        duk_remove(ctx, index);

    return {
        std::move(name),
        std::move(message),
        std::move(stack),
        std::move(file_name),
        line_number
    };
}

} // !irccd

#endif // !IRCCD_JS_DUKTAPE_HPP

/*
 * duktape.hpp -- Duktape extras
 *
 * Copyright (c) 2016-2017 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DUKTAPE_HPP
#define IRCCD_DUKTAPE_HPP

/**
 * \file duktape.hpp
 * \brief Bring some extras to Duktape C library.
 * \author David Demelier <markand@malikania.fr>
 */

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <duktape.h>

namespace irccd {

/**
 * \brief Stack sanity checker.
 *
 * Instanciate this class where you need to manipulate the Duktape stack outside a Duktape/C function, its destructor
 * will examinate if the stack size matches the user expected size.
 *
 * When compiled with NDEBUG, this class does nothing.
 *
 * To use it, just declare an lvalue at the beginning of your function.
 */
class StackAssert {
#if !defined(NDEBUG)
private:
    duk_context *m_context;
    unsigned m_expected;
    unsigned m_begin;
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
    inline StackAssert(duk_context *ctx, unsigned expected = 0) noexcept
#if !defined(NDEBUG)
        : m_context(ctx)
        , m_expected(expected)
        , m_begin(static_cast<unsigned>(duk_get_top(ctx)))
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
    inline ~StackAssert() noexcept
    {
#if !defined(NDEBUG)
        if (static_cast<unsigned>(duk_get_top(m_context)) - m_begin != m_expected) {
            std::fprintf(stderr, "Corrupt stack detection in StackAssert:\n");
            std::fprintf(stderr, "  Size at start:            %u\n", m_begin);
            std::fprintf(stderr, "  Size at end:              %d\n", duk_get_top(m_context));
            std::fprintf(stderr, "  Expected (user):          %u\n", m_expected);
            std::fprintf(stderr, "  Expected (adjusted):      %u\n", m_expected + m_begin);
            std::fprintf(stderr, "  Number of stale values:   %u\n", duk_get_top(m_context) - m_begin - m_expected);
            std::abort();
        }
#endif
    }
};

/**
 * \brief Error description.
 *
 * This class fills the fields got in an Error object.
 */
class Exception : public std::exception {
public:
    std::string name;        //!< name of error
    std::string message;        //!< error message
    std::string stack;        //!< stack if available
    std::string fileName;        //!< filename if applicable
    int lineNumber{0};        //!< line number if applicable

    /**
     * Get the error message. This effectively returns message field.
     *
     * \return the message
     */
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};

/**
 * \brief RAII based Duktape handler.
 *
 * This class is implicitly convertible to duk_context for convenience.
 */
class UniqueContext {
private:
    using Deleter = void (*)(duk_context *);
    using Handle = std::unique_ptr<duk_context, Deleter>;

    Handle m_handle;

    UniqueContext(const UniqueContext &) = delete;
    UniqueContext &operator=(const UniqueContext &) = delete;

public:
    /**
     * Create default context.
     */
    inline UniqueContext()
        : m_handle(duk_create_heap_default(), duk_destroy_heap)
    {
    }

    /**
     * Default move constructor.
     */
    UniqueContext(UniqueContext &&) noexcept = default;

    /**
     * Convert the context to the native Duktape/C type.
     *
     * \return the duk_context
     */
    inline operator duk_context *() noexcept
    {
        return m_handle.get();
    }

    /**
     * Convert the context to the native Duktape/C type.
     *
     * \return the duk_context
     */
    inline operator duk_context *() const noexcept
    {
        return m_handle.get();
    }

    /**
     * Default move assignment operator.
     *
     * \return this
     */
    UniqueContext &operator=(UniqueContext &&) noexcept = delete;
};

/**
 * \brief Base ECMAScript error class.
 * \warning Override the function create for your own exceptions
 */
class Error {
private:
    int m_type{DUK_ERR_ERROR};
    std::string m_message;

protected:
    /**
     * Constructor with a type of error specified, specially designed for derived errors.
     *
     * \param type of error (e.g. DUK_ERR_ERROR)
     * \param message the message
     */
    inline Error(int type, std::string message) noexcept
        : m_type(type)
        , m_message(std::move(message))
    {
    }

public:
    /**
     * Constructor with a message.
     *
     * \param message the message
     */
    inline Error(std::string message) noexcept
        : m_message(std::move(message))
    {
    }

    /**
     * Create the exception on the stack.
     *
     * \note the default implementation search for the global variables
     * \param ctx the context
     */
    virtual void raise(duk_context *ctx) const
    {
        duk_error(ctx, m_type, "%s", m_message.c_str());
    }
};

/**
 * \brief Error in eval() function.
 */
class EvalError : public Error {
public:
    /**
     * Construct an EvalError.
     *
     * \param message the message
     */
    inline EvalError(std::string message) noexcept
        : Error(DUK_ERR_EVAL_ERROR, std::move(message))
    {
    }
};

/**
 * \brief Value is out of range.
 */
class RangeError : public Error {
public:
    /**
     * Construct an RangeError.
     *
     * \param message the message
     */
    inline RangeError(std::string message) noexcept
        : Error(DUK_ERR_RANGE_ERROR, std::move(message))
    {
    }
};

/**
 * \brief Trying to use a variable that does not exist.
 */
class ReferenceError : public Error {
public:
    /**
     * Construct an ReferenceError.
     *
     * \param message the message
     */
    inline ReferenceError(std::string message) noexcept
        : Error(DUK_ERR_REFERENCE_ERROR, std::move(message))
    {
    }
};

/**
 * \brief Syntax error in the script.
 */
class SyntaxError : public Error {
public:
    /**
     * Construct an SyntaxError.
     *
     * \param message the message
     */
    inline SyntaxError(std::string message) noexcept
        : Error(DUK_ERR_SYNTAX_ERROR, std::move(message))
    {
    }
};

/**
 * \brief Invalid type given.
 */
class TypeError : public Error {
public:
    /**
     * Construct an TypeError.
     *
     * \param message the message
     */
    inline TypeError(std::string message) noexcept
        : Error(DUK_ERR_TYPE_ERROR, std::move(message))
    {
    }
};

/**
 * \brief URI manipulation failure.
 */
class URIError : public Error {
public:
    /**
     * Construct an URIError.
     *
     * \param message the message
     */
    inline URIError(std::string message) noexcept
        : Error(DUK_ERR_URI_ERROR, std::move(message))
    {
    }
};

/**
 * Get the error object when a JavaScript error has been thrown (e.g. eval failure).
 *
 * \param ctx the context
 * \param index the index
 * \param pop if true, also remove the exception from the stack
 * \return the information
 */
inline Exception dukx_exception(duk_context *ctx, int index, bool pop = true)
{
    Exception ex;

    index = duk_normalize_index(ctx, index);

    duk_get_prop_string(ctx, index, "name");
    ex.name = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "message");
    ex.message = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "fileName");
    ex.fileName = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "lineNumber");
    ex.lineNumber = duk_to_int(ctx, -1);
    duk_get_prop_string(ctx, index, "stack");
    ex.stack = duk_to_string(ctx, -1);
    duk_pop_n(ctx, 5);

    if (pop)
        duk_remove(ctx, index);

    return ex;
}

/**
 * Enumerate an object or an array at the specified index.
 *
 * \param ctx the context
 * \param index the object or array index
 * \param flags the optional flags to pass to duk_enum
 * \param getvalue set to true if you want to extract the value
 * \param func the function to call for each properties
 */
template <typename Func>
void dukx_enumerate(duk_context *ctx, int index, duk_uint_t flags, duk_bool_t getvalue, Func &&func)
{
    duk_enum(ctx, index, flags);

    while (duk_next(ctx, -1, getvalue)) {
        func(ctx);
        duk_pop_n(ctx, 1 + (getvalue ? 1 : 0));
    }

    duk_pop(ctx);
}

/**
 * Throw an ECMAScript exception.
 *
 * \param ctx the context
 * \param ex the exception
 */
template <typename Exception>
void dukx_throw(duk_context *ctx, const Exception &ex)
{
    ex.raise(ctx);
}

/**
 * Get a string, return 0 if not a string.
 *
 * \param ctx the context
 * \param index the index
 * \return the string
 */
inline std::string dukx_get_std_string(duk_context *ctx, int index)
{
    duk_size_t size;
    const char *text = duk_get_lstring(ctx, index, &size);

    return std::string(text, size);
}

/**
 * Require a string, throws a JavaScript exception if not a string.
 *
 * \param ctx the context
 * \param index the index
 * \return the string
 */
inline std::string dukx_require_std_string(duk_context *ctx, int index)
{
    duk_size_t size;
    const char *text = duk_require_lstring(ctx, index, &size);

    return std::string(text, size);
}

/**
 * Push a C++ string.
 *
 * \param ctx the context
 * \param str the string
 */
inline void dukx_push_std_string(duk_context *ctx, const std::string &str)
{
    duk_push_lstring(ctx, str.data(), str.length());
}

/**
 * Get an array.
 *
 * \param ctx the context
 * \param index the array index
 * \param get the conversion function (e.g. duk_get_int)
 */
template <typename Getter>
auto dukx_get_array(duk_context *ctx, duk_idx_t index, Getter &&get)
{
    using T = decltype(get(ctx, 0));

    std::vector<T> result;
    std::size_t length = duk_get_length(ctx, index);

    for (std::size_t i = 0; i < length; ++i) {
        duk_get_prop_index(ctx, -1, i);
        result.push_back(get(ctx, -1));
        duk_pop(ctx);
    }

    return result;
}

/**
 * Push an array.
 *
 * \param ctx the context
 * \param values the values
 * \param push the function to push values
 */
template <typename T, typename Pusher>
void dukx_push_array(duk_context *ctx, const std::vector<T> &values, Pusher &&push)
{
    duk_push_array(ctx);

    int i = 0;
    for (auto x : values) {
        push(ctx, x);
        duk_put_prop_index(ctx, -2, i++);
    }
}

} // !irccd

#endif // !IRCCD_DUKTAPE_HPP

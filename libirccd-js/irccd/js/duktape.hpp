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

#ifndef IRCCD_JS_DUKTAPE_HPP
#define IRCCD_JS_DUKTAPE_HPP

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
 * \brief Error description.
 *
 * This class fills the fields got in an Error object.
 */
class dukx_exception : public std::exception {
public:
    std::string name;               //!< name of error
    std::string message;            //!< error message
    std::string stack;              //!< stack if available
    std::string file_name;          //!< filename if applicable
    int line_number{0};             //!< line number if applicable

    /**
     * Get the error message. This effectively returns message field.
     *
     * \return the message
     */
    const char* what() const noexcept override
    {
        return message.c_str();
    }
};

/**
 * \brief RAII based Duktape handler.
 *
 * This class is implicitly convertible to duk_context for convenience.
 */
class dukx_context {
private:
    std::unique_ptr<duk_context, void (*)(duk_context*)> m_handle;

    dukx_context(const dukx_context&) = delete;
    dukx_context &operator=(const dukx_context&) = delete;

public:
    /**
     * Create default context.
     */
    inline dukx_context() noexcept
        : m_handle(duk_create_heap_default(), duk_destroy_heap)
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
        return m_handle.get();
    }

    /**
     * Convert the context to the native Duktape/C type.
     *
     * \return the duk_context
     */
    inline operator duk_context*() const noexcept
    {
        return m_handle.get();
    }

    /**
     * Default move assignment operator.
     *
     * \return this
     */
    dukx_context& operator=(dukx_context&&) noexcept = delete;
};

/**
 * Get the error object when a JavaScript error has been thrown (e.g. eval
 * failure).
 *
 * \param ctx the context
 * \param index the index
 * \param pop if true, also remove the exception from the stack
 * \return the information
 */
inline dukx_exception dukx_get_exception(duk_context* ctx, int index, bool pop = true)
{
    dukx_exception ex;

    index = duk_normalize_index(ctx, index);

    duk_get_prop_string(ctx, index, "name");
    ex.name = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "message");
    ex.message = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "fileName");
    ex.file_name = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "lineNumber");
    ex.line_number = duk_to_int(ctx, -1);
    duk_get_prop_string(ctx, index, "stack");
    ex.stack = duk_to_string(ctx, -1);
    duk_pop_n(ctx, 5);

    if (pop)
        duk_remove(ctx, index);

    return ex;
}

/**
 * Get a string, return 0 if not a string.
 *
 * \param ctx the context
 * \param index the index
 * \return the string
 */
inline std::string dukx_get_string(duk_context* ctx, int index)
{
    duk_size_t size;
    const char* text = duk_get_lstring(ctx, index, &size);

    return std::string(text, size);
}

/**
 * Require a string, throws a JavaScript exception if not a string.
 *
 * \param ctx the context
 * \param index the index
 * \return the string
 */
inline std::string dukx_require_string(duk_context* ctx, int index)
{
    duk_size_t size;
    const char* text = duk_require_lstring(ctx, index, &size);

    return std::string(text, size);
}

/**
 * Push a C++ string.
 *
 * \param ctx the context
 * \param str the string
 */
inline void dukx_push_string(duk_context* ctx, const std::string& str)
{
    duk_push_lstring(ctx, str.data(), str.length());
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

} // !irccd

#endif // !MALIKANIA_COMMON_DUKTAPE_HPP

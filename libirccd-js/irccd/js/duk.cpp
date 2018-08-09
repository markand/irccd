/*
 * duk.cpp -- miscellaneous Duktape extras
 *
 * Copyright (c) 2017-2018 David Demelier <markand@malikania.fr>
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

#include <cassert>
#include <cstdio>

#include "duk.hpp"

namespace irccd::js::duk {

// {{{ stack_guard

stack_guard::stack_guard(duk_context* ctx, unsigned expected) noexcept
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

stack_guard::~stack_guard() noexcept
{
#if !defined(NDEBUG)
    auto result = duk_get_top(context_) - at_start_;

    if (result != static_cast<int>(expected_)) {
        std::fprintf(stderr, "Corrupt stack detection in stack_guard:\n");
        std::fprintf(stderr, "  Size at start:           %d\n", at_start_);
        std::fprintf(stderr, "  Size at end:             %d\n", duk_get_top(context_));
        std::fprintf(stderr, "  Expected (user):         %u\n", expected_);
        std::fprintf(stderr, "  Expected (adjusted):     %u\n", expected_ + at_start_);
        std::fprintf(stderr, "  Difference count:       %+d\n", result - expected_);
        std::abort();
    }
#endif
}

// }}}

// {{{ context

context::context() noexcept
    : handle_(duk_create_heap_default(), duk_destroy_heap)
{
}

context::operator duk_context*() noexcept
{
    return handle_.get();
}

context::operator duk_context*() const noexcept
{
    return handle_.get();
}

// }}}

// {{{ stack_info

stack_info::stack_info(std::string name,
                       std::string message,
                       std::string stack,
                       std::string file_name,
                       unsigned line_number) noexcept
    : name_(std::move(name))
    , message_(std::move(message))
    , stack_(std::move(stack))
    , file_name_(std::move(file_name))
    , line_number_(line_number)
{
}

auto stack_info::get_name() const noexcept -> const std::string&
{
    return name_;
}

auto stack_info::get_message() const noexcept -> const std::string&
{
    return message_;
}

auto stack_info::get_stack() const noexcept -> const std::string&
{
    return stack_;
}

auto stack_info::get_file_name() const noexcept -> const std::string&
{
    return file_name_;
}

auto stack_info::get_line_number() const noexcept -> unsigned
{
    return line_number_;
}

auto stack_info::what() const noexcept -> const char*
{
    return message_.c_str();
}

// }}}

// {{{ error

error::error(int type, std::string message) noexcept
    : type_(type)
    , message_(std::move(message))
{
}

error::error(std::string message) noexcept
    : message_(std::move(message))
{
}

void error::create(duk_context* ctx) const
{
    duk_push_error_object(ctx, type_, "%s", message_.c_str());
}

// }}}

// {{{ eval_error

eval_error::eval_error(std::string message) noexcept
    : error(DUK_ERR_EVAL_ERROR, std::move(message))
{
}

// }}}

// {{{ range_error

range_error::range_error(std::string message) noexcept
    : error(DUK_ERR_RANGE_ERROR, std::move(message))
{
}

// }}}

// {{{ reference_error

reference_error::reference_error(std::string message) noexcept
    : error(DUK_ERR_REFERENCE_ERROR, std::move(message))
{
}

// }}}

// {{{ syntax_error

syntax_error::syntax_error(std::string message) noexcept
    : error(DUK_ERR_SYNTAX_ERROR, std::move(message))
{
}

// }}}

// {{{ type_error

type_error::type_error(std::string message) noexcept
    : error(DUK_ERR_TYPE_ERROR, std::move(message))
{
}

// }}}

// {{{ uri_error

uri_error::uri_error(std::string message) noexcept
    : error(DUK_ERR_URI_ERROR, std::move(message))
{
}

// }}}

// {{{ get_stack

auto get_stack(duk_context* ctx, int index, bool pop) -> stack_info
{
    index = duk_normalize_index(ctx, index);

    duk_get_prop_string(ctx, index, "name");
    auto name = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "message");
    auto message = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "fileName");
    auto file_name = duk_to_string(ctx, -1);
    duk_get_prop_string(ctx, index, "lineNumber");
    auto line_number = duk_to_uint(ctx, -1);
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

// }}}

// {{{ type_traits<std::exception>

void type_traits<std::exception>::raise(duk_context* ctx, const std::exception& ex)
{
    duk_error(ctx, DUK_ERR_ERROR, "%s", ex.what());
}

// }}}

// {{{ type_traits<error>

void type_traits<error>::raise(duk_context* ctx, const error& ex)
{
    ex.create(ctx);
    duk_throw(ctx);
}

// }}}

// {{{ type_traits<bool>

void type_traits<bool>::push(duk_context* ctx, bool value)
{
    duk_push_boolean(ctx, value);
}

auto type_traits<bool>::get(duk_context* ctx, duk_idx_t index) -> bool
{
    return duk_get_boolean(ctx, index);
}

auto type_traits<bool>::require(duk_context* ctx, duk_idx_t index) -> bool
{
    return duk_require_boolean(ctx, index);
}

// }}}

// {{{ type_traits<duk_double_t>

void type_traits<duk_double_t>::push(duk_context* ctx, duk_double_t value)
{
    duk_push_number(ctx, value);
}

auto type_traits<duk_double_t>::get(duk_context* ctx, duk_idx_t index) -> duk_double_t
{
    return duk_get_number(ctx, index);
}

auto type_traits<duk_double_t>::require(duk_context* ctx, duk_idx_t index) -> duk_double_t
{
    return duk_require_number(ctx, index);
}

// }}}

// {{{ type_traits<duk_int_t>

void type_traits<duk_int_t>::push(duk_context* ctx, duk_int_t value)
{
    duk_push_int(ctx, value);
}

auto type_traits<duk_int_t>::get(duk_context* ctx, duk_idx_t index) -> duk_int_t
{
    return duk_get_int(ctx, index);
}

auto type_traits<duk_int_t>::require(duk_context* ctx, duk_idx_t index) -> duk_int_t
{
    return duk_require_int(ctx, index);
}

// }}}

// {{{ type_traits<duk_uint_t>

void type_traits<duk_uint_t>::push(duk_context* ctx, duk_uint_t value)
{
    duk_push_uint(ctx, value);
}

auto type_traits<duk_uint_t>::get(duk_context* ctx, duk_idx_t index) -> duk_uint_t
{
    return duk_get_uint(ctx, index);
}

auto type_traits<duk_uint_t>::require(duk_context* ctx, duk_idx_t index) -> duk_uint_t
{
    return duk_require_uint(ctx, index);
}

// }}}

// {{{ type_traits<const char*>

void type_traits<const char*>::push(duk_context* ctx, const char* value)
{
    duk_push_string(ctx, value);
}

auto type_traits<const char*>::get(duk_context* ctx, duk_idx_t index) -> const char*
{
    return duk_get_string(ctx, index);
}

auto type_traits<const char*>::require(duk_context* ctx, duk_idx_t index) -> const char*
{
    return duk_require_string(ctx, index);
}

// }}}

// {{{ type_traits<std::string>

void type_traits<std::string>::push(duk_context* ctx, const std::string& value)
{
    duk_push_lstring(ctx, value.data(), value.size());
}

auto type_traits<std::string>::get(duk_context* ctx, duk_idx_t index) -> std::string
{
    duk_size_t length;
    const char* str = duk_get_lstring(ctx, index, &length);

    return { str, length };
}

auto type_traits<std::string>::require(duk_context* ctx, duk_idx_t index) -> std::string
{
    duk_size_t length;
    const char* str = duk_require_lstring(ctx, index, &length);

    return { str, length };
}

// }}}

// {{{ type_traits<std::string_view>

void type_traits<std::string_view>::push(duk_context* ctx, std::string_view value)
{
    duk_push_lstring(ctx, value.data(), value.size());
}

auto type_traits<std::string_view>::get(duk_context* ctx, duk_idx_t index) -> std::string_view
{
    duk_size_t length;
    const char* str = duk_get_lstring(ctx, index, &length);

    return { str, length };
}

auto type_traits<std::string_view>::require(duk_context* ctx, duk_idx_t index) -> std::string_view
{
    duk_size_t length;
    const char* str = duk_require_lstring(ctx, index, &length);

    return { str, length };
}

// }}}

} // !irccd::js::duk

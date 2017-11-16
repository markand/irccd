/*
 * alias.cpp -- create irccdctl aliases
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

#include <cassert>
#include <regex>

#include "alias.hpp"

namespace irccd {

namespace ctl {

alias_arg::alias_arg(std::string value)
{
    assert(!value.empty());

    if ((is_placeholder_ = std::regex_match(value, std::regex("^%\\d+$"))))
        value_ = value.substr(1);
    else
        value_ = std::move(value);
}

unsigned alias_arg::index() const noexcept
{
    assert(is_placeholder_);

    return std::stoi(value_);
}

const std::string& alias_arg::value() const noexcept
{
    assert(!is_placeholder_);

    return value_;
}

std::ostream& operator<<(std::ostream& out, const alias_arg& arg)
{
    if (arg.is_placeholder_)
        out << "%" << arg.value_;
    else
        out << arg.value_;

    return out;
}

} // !ctl

} // !irccd

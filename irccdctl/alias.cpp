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

AliasArg::AliasArg(std::string value)
{
    assert(!value.empty());

    if ((m_isPlaceholder = std::regex_match(value, std::regex("^%\\d+$"))))
        m_value = value.substr(1);
    else
        m_value = std::move(value);
}

unsigned AliasArg::index() const noexcept
{
    assert(isPlaceholder());

    return std::stoi(m_value);
}

const std::string &AliasArg::value() const noexcept
{
    assert(!isPlaceholder());

    return m_value;
}

std::ostream &operator<<(std::ostream &out, const AliasArg &arg)
{
    if (arg.m_isPlaceholder)
        out << "%" << arg.m_value;
    else
        out << arg.m_value;

    return out;
}

} // !irccd

/*
 * rule.cpp -- rule for server and channels
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

#include <algorithm>
#include <cctype>

#include "rule.hpp"

namespace irccd {

bool rule::match_set(const set& set, const std::string& value) const noexcept
{
    return value.empty() || set.empty() || set.count(value) == 1;
}

rule::rule(set servers, set channels, set origins, set plugins, set events, action_type action)
    : servers_(std::move(servers))
    , channels_(std::move(channels))
    , origins_(std::move(origins))
    , plugins_(std::move(plugins))
    , events_(std::move(events))
    , action_(action)
{
}

bool rule::match(const std::string& server,
                 const std::string& channel,
                 const std::string& nick,
                 const std::string& plugin,
                 const std::string& event) const noexcept
{
    auto tolower = [] (auto str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    };

    return match_set(servers_, tolower(server)) &&
           match_set(channels_, tolower(channel)) &&
           match_set(origins_, tolower(nick)) &&
           match_set(plugins_, tolower(plugin)) &&
           match_set(events_, event);
}

const boost::system::error_category& rule_category()
{
    static const class category : public boost::system::error_category {
    public:
        const char* name() const noexcept override
        {
            return "rule";
        }

        std::string message(int e) const override
        {
            switch (static_cast<rule_error::error>(e)) {
            case rule_error::invalid_action:
                return "invalid action given";
            case rule_error::invalid_index:
                return "invalid index";
            default:
                return "no error";
            }
        }
    } category;

    return category;
}

boost::system::error_code make_error_code(rule_error::error e)
{
    return {static_cast<int>(e), rule_category()};
}

} // !irccd

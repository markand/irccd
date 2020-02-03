/*
 * rule.cpp -- rule for server and channels
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

#include <irccd/sysconfig.hpp>

#include <algorithm>
#include <cctype>

#include "rule.hpp"

namespace irccd::daemon {

auto rule::match_set(const set& set, const std::string& value) const noexcept -> bool
{
	return set.empty() || set.count(value) == 1;
}

auto rule::match(std::string_view server,
                 std::string_view channel,
                 std::string_view nick,
                 std::string_view plugin,
                 std::string_view event) const noexcept -> bool
{
	const auto tolower = [] (auto str) noexcept -> std::string {
		std::string ret(str);
		std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
		return ret;
	};

	return match_set(servers, tolower(server)) &&
	       match_set(channels, tolower(channel)) &&
	       match_set(origins, tolower(nick)) &&
	       match_set(plugins, tolower(plugin)) &&
	       match_set(events, std::string(event));
}

auto operator==(const rule& lhs, const rule& rhs) noexcept -> bool
{
	return lhs.servers == rhs.servers &&
	       lhs.channels == rhs.channels &&
	       lhs.origins == rhs.origins &&
	       lhs.plugins == rhs.plugins &&
	       lhs.events == rhs.events &&
	       lhs.action == rhs.action;
}

auto operator!=(const rule& lhs, const rule& rhs) noexcept -> bool
{
	return !(lhs == rhs);
}

auto rule_category() -> const std::error_category&
{
	static const class category : public std::error_category {
	public:
		auto name() const noexcept -> const char* override
		{
			return "rule";
		}

		auto message(int e) const -> std::string override
		{
			switch (static_cast<rule_error::error>(e)) {
			case rule_error::invalid_action:
				return "invalid rule action";
			case rule_error::invalid_index:
				return "invalid rule index";
			default:
				return "no error";
			}
		}
	} category;

	return category;
}

auto make_error_code(rule_error::error e) -> std::error_code
{
	return { static_cast<int>(e), rule_category() };
}

} // !irccd::daemon

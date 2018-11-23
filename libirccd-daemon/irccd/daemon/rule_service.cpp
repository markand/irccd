/*
 * rule_service.cpp -- rule service
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

#include <irccd/sysconfig.hpp>

#include <stdexcept>

#include <irccd/config.hpp>
#include <irccd/string_util.hpp>

#include "bot.hpp"
#include "logger.hpp"
#include "rule_service.hpp"
#include "rule_util.hpp"

namespace irccd::daemon {

rule_service::rule_service(bot& bot)
	: bot_(bot)
{
}

auto rule_service::list() const noexcept -> const std::vector<rule>&
{
	return rules_;
}

void rule_service::add(rule rule)
{
	rules_.push_back(std::move(rule));
}

void rule_service::insert(rule rule, unsigned position)
{
	assert(position <= rules_.size());

	rules_.insert(rules_.begin() + position, std::move(rule));
}

void rule_service::remove(unsigned position)
{
	assert(position < rules_.size());

	rules_.erase(rules_.begin() + position);
}

auto rule_service::require(unsigned position) const -> const rule&
{
	if (position >= rules_.size())
		throw rule_error(rule_error::invalid_index);

	return rules_[position];
}

auto rule_service::require(unsigned position) -> rule&
{
	if (position >= rules_.size())
		throw rule_error(rule_error::invalid_index);

	return rules_[position];
}

auto rule_service::solve(std::string_view server,
                         std::string_view channel,
                         std::string_view origin,
                         std::string_view plugin,
                         std::string_view event) noexcept -> bool
{
	bool result = true;

	for (const auto& rule : rules_)
		if (rule.match(server, channel, origin, plugin, event))
			result = rule.action == rule::action_type::accept;

	return result;
}

void rule_service::load(const config& cfg) noexcept
{
	rules_.clear();

	for (const auto& section : cfg) {
		if (section.get_key() != "rule")
			continue;

		try {
			rules_.push_back(rule_util::from_config(section));
		} catch (const std::exception& ex) {
			bot_.get_log().warning("rule", "") << ex.what() << std::endl;
		}
	}
}

namespace logger {

auto type_traits<rule>::get_category(const rule&) -> std::string_view
{
	return "rule";
}

auto type_traits<rule>::get_component(const rule&) -> std::string_view
{
	return "";
}

} // !logger

} // !irccd::daemon

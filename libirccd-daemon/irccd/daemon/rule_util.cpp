/*
 * rule_util.cpp -- rule utilities
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

#include <irccd/ini.hpp>

#include "rule.hpp"
#include "rule_util.hpp"

namespace irccd::daemon::rule_util {

auto from_config(const ini::section& sc) -> rule
{
	// Simple converter from std::vector to std::unordered_set.
	const auto toset = [] (const auto& v) {
		return std::set<std::string>(v.begin(), v.end());
	};

	rule::set servers, channels, origins, plugins, events;
	rule::action_type action = rule::action_type::accept;

	// Get the sets.
	ini::section::const_iterator it;

	if ((it = sc.find("servers")) != sc.end())
		servers = toset(*it);
	if ((it = sc.find("channels")) != sc.end())
		channels = toset(*it);
	if ((it = sc.find("origins")) != sc.end())
		origins = toset(*it);
	if ((it = sc.find("plugins")) != sc.end())
		plugins = toset(*it);
	if ((it = sc.find("channels")) != sc.end())
		channels = toset(*it);
	if ((it = sc.find("events")) != sc.end())
		events = toset(*it);

	// Get the action.
	auto actionstr = sc.get("action").get_value();

	if (actionstr == "drop")
		action = rule::action_type::drop;
	else if (actionstr == "accept")
		action = rule::action_type::accept;
	else
		throw rule_error(rule_error::invalid_action);

	return {
		std::move(servers),
		std::move(channels),
		std::move(origins),
		std::move(plugins),
		std::move(events),
		action
	};
}

auto from_json(const nlohmann::json& json) -> rule
{
	const auto toset = [] (auto object, auto name) {
		rule::set result;

		for (const auto& s : object[name])
			if (s.is_string())
				result.insert(s.template get<std::string>());

		return result;
	};
	const auto toaction = [] (const auto& object, const auto& name) {
		const auto v = object.find(name);

		if (v == object.end() || !v->is_string())
			throw rule_error(rule_error::invalid_action);

		const auto s = v->template get<std::string>();

		if (s == "accept")
			return rule::action_type::accept;
		if (s == "drop")
			return rule::action_type::drop;

		throw rule_error(rule_error::invalid_action);
	};

	return {
		toset(json, "servers"),
		toset(json, "channels"),
		toset(json, "origins"),
		toset(json, "plugins"),
		toset(json, "events"),
		toaction(json, "action")
	};
}

auto get_index(const nlohmann::json& json, const std::string& key) -> unsigned
{
	const auto index = json.find(key);

	if (index == json.end() || !index->is_number_unsigned())
		throw rule_error(rule_error::invalid_index);

	return index->get<unsigned>();
}

auto to_json(const rule& rule) -> nlohmann::json
{
	const auto join = [] (const auto& set) {
		auto array = nlohmann::json::array();

		for (const auto& entry : set)
			array.push_back(entry);

		return array;
	};
	const auto str = [] (auto action) {
		switch (action) {
		case rule::action_type::accept:
			return "accept";
		default:
			return "drop";
		}
	};

	return {
		{ "servers",    join(rule.servers)      },
		{ "channels",   join(rule.channels)     },
		{ "origins",    join(rule.origins)      },
		{ "plugins",    join(rule.plugins)      },
		{ "events",     join(rule.events)       },
		{ "action",     str(rule.action)        }
	};
}

} // !irccd::daemon::rule_util

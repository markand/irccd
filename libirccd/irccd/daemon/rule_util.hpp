/*
 * rule_util.hpp -- rule utilities
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

#ifndef IRCCD_DAEMON_RULE_UTIL_HPP
#define IRCCD_DAEMON_RULE_UTIL_HPP

/**
 * \file rule_util.hpp
 * \brief Rule utilities.
 */

#include <json.hpp>

namespace irccd {

namespace ini {

class section;

} // !ini

class config;

struct rule;

/**
 * \brief Rule utilities.
 */
namespace rule_util {

/**
 * Load a rule from a JSON object.
 *
 * For possible use in transport commands or Javascript API.
 *
 * \pre json.is_object()
 * \param json the JSON object
 * \return the new rule
 * \throw rule_error on errors
 */
auto from_json(const nlohmann::json& json) -> rule;

/**
 * Load a rule from a INI section.
 *
 * \param sc the ini section
 * \return the rule
 * \throw rule_error on errors
 */
auto from_config(const ini::section& sc) -> rule;

/**
 * Convert a rule into a JSON object.
 *
 * \param rule the rule
 * \throw the JSON representation
 * \return the JSON representation
 */
auto to_json(const rule& rule) -> nlohmann::json;

} // !rule_util

} // !irccd

#endif // !IRCCD_DAEMON_RULE_UTIL_HPP

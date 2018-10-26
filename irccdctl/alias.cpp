/*
 * alias.cpp -- create irccdctl aliases
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

auto alias_arg::is_placeholder() const noexcept -> bool
{
	return is_placeholder_;
}

unsigned alias_arg::get_index() const noexcept
{
	assert(is_placeholder_);

	return std::stoi(value_);
}

const std::string& alias_arg::get_value() const noexcept
{
	assert(!is_placeholder_);

	return value_;
}

std::ostream& operator<<(std::ostream& out, const alias_arg& arg)
{
	if (arg.is_placeholder())
		out << "%" << arg.get_value();
	else
		out << arg.get_value();

	return out;
}

alias_command::alias_command(std::string command, std::vector<alias_arg> args) noexcept
	: command_(std::move(command))
	, args_(std::move(args))
{
}

auto alias_command::get_command() const noexcept -> const std::string&
{
	return command_;
}

auto alias_command::get_args() const noexcept -> const std::vector<alias_arg>&
{
	return args_;
}

alias::alias(std::string name) noexcept
	: name_(std::move(name))
{
}

auto alias::get_name() const noexcept -> const std::string&
{
	return name_;
}

} // !ctl

} // !irccd

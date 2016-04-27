/*
 * command.cpp -- remote command
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <iomanip>
#include <numeric>
#include <sstream>

#include "command.hpp"
#include "logger.hpp"
#include "system.hpp"

using namespace std::string_literals;

namespace irccd {

std::string RemoteCommand::usage() const
{
	std::ostringstream oss;

	oss << "usage: " << sys::programName() << " " << m_name;

	/* Options summary */
	if (options().size() > 0) {
		oss << " [options...]";
	}

	/* Arguments summary */
	if (args().size() > 0) {
		oss << " ";

		for (const auto &arg : args()) {
			oss << (arg.required() ? "" : "[") << arg.name() << (arg.required() ? "" : "]") << " ";
		}
	}

	/* Description */
	oss << "\n\n" << help() << "\n\n";

	/* Options */
	if (options().size() > 0) {
		oss << "Options:\n";

		for (const auto &opt : options()) {
			std::ostringstream optoss;

			/* Construct the line for the option in a single string to pad it correctly */
			optoss << "  ";
			optoss << (!opt.simpleKey().empty() ? ("-"s + opt.simpleKey() + " ") : "   ");
			optoss << (!opt.longKey().empty() ? ("--"s + opt.longKey() + " "s) : "");
			optoss << opt.arg();

			/* Add it padded with spaces */
			oss << std::left << std::setw(28) << optoss.str();
			oss << opt.description() << "\n";
		}
	}

	return oss.str();
}

unsigned RemoteCommand::min() const noexcept
{
	auto list = args();

	return std::accumulate(list.begin(), list.end(), 0U, [] (unsigned i, const auto &arg) noexcept -> unsigned {
		return i + (arg.required() ? 1 : 0);
	});
}

unsigned RemoteCommand::max() const noexcept
{
	return (unsigned)args().size();
}

json::Value RemoteCommand::request(Irccdctl &, const RemoteCommandRequest &) const
{
	return json::object({});
}

json::Value RemoteCommand::exec(Irccd &, const json::Value &) const
{
	return json::object({});
}

void RemoteCommand::result(Irccdctl &, const json::Value &response) const
{
	auto it = response.find("error");

	if (it != response.end() && it->isString()) {
		log::warning() << "irccdctl: " << it->toString() << std::endl;
	}
}

} // !irccd

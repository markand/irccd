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

#include <format.h>

#include "command.hpp"
#include "logger.hpp"
#include "system.hpp"

using namespace std::string_literals;

using namespace fmt::literals;

namespace irccd {

namespace {

/*
 * typeName
 * ------------------------------------------------------------------
 *
 * Convert a JSON value type to string for convenience.
 */
std::string typeName(json::Type type)
{
	static const std::vector<std::string> typenames{
		"array", "boolean", "int", "null", "object", "real", "string"
	};

	assert(type >= json::Type::Array && type <= json::Type::String);

	return typenames[static_cast<int>(type)];
}

/*
 * typeNameList
 * ------------------------------------------------------------------
 *
 * Construct a list of names to send a convenient error message if properties are invalid, example: string, int or bool expected.
 */

std::string typeNameList(const std::vector<json::Type> &types)
{
	std::ostringstream oss;

	if (types.size() == 1)
		return typeName(types[0]);

	for (std::size_t i = 0; i < types.size(); ++i) {
		oss << typeName(types[i]);

		if (i == types.size() - 2)
			oss << " or ";
		else if (i < types.size() - 1)
			oss << ", ";
	}

	return oss.str();
}

} // !namespace

std::string Command::usage() const
{
	std::ostringstream oss;

	oss << "usage: " << sys::programName() << " " << m_name;

	// Options summary.
	if (options().size() > 0)
		oss << " [options...]";

	// Arguments summary.
	if (args().size() > 0) {
		oss << " ";

		for (const auto &arg : args())
			oss << (arg.required() ? "" : "[") << arg.name() << (arg.required() ? "" : "]") << " ";
	}

	// Description.
	oss << "\n\n" << help() << "\n\n";

	// Options.
	if (options().size() > 0) {
		oss << "Options:\n";

		for (const auto &opt : options()) {
			std::ostringstream optoss;

			// Construct the line for the option in a single string to pad it correctly.
			optoss << "  ";
			optoss << (!opt.simpleKey().empty() ? ("-"s + opt.simpleKey() + " ") : "   ");
			optoss << (!opt.longKey().empty() ? ("--"s + opt.longKey() + " "s) : "");
			optoss << opt.arg();

			// Add it padded with spaces.
			oss << std::left << std::setw(28) << optoss.str();
			oss << opt.description() << "\n";
		}
	}

	return oss.str();
}

unsigned Command::min() const noexcept
{
	auto list = args();

	return std::accumulate(list.begin(), list.end(), 0U, [] (unsigned i, const auto &arg) noexcept -> unsigned {
		return i + (arg.required() ? 1 : 0);
	});
}

unsigned Command::max() const noexcept
{
	return (unsigned)args().size();
}

json::Value Command::request(Irccdctl &, const CommandRequest &) const
{
	return json::object({});
}

json::Value Command::exec(Irccd &, const json::Value &request) const
{
	// Verify that requested properties are present in the request.
	for (const auto &prop : properties()) {
		auto it = request.find(prop.name());

		if (it == request.end())
			throw std::invalid_argument("missing '{}' property"_format(prop.name()));

		if (std::find(prop.types().begin(), prop.types().end(), it->typeOf()) == prop.types().end()) {
			auto expected = typeNameList(prop.types());
			auto got = typeName(it->typeOf());

			throw std::invalid_argument("invalid '{}' property ({} expected, got {})"_format(prop.name(), expected, got));
		}
	}

	return json::object({});
}

void Command::result(Irccdctl &, const json::Value &response) const
{
	auto it = response.find("error");

	if (it != response.end() && it->isString())
		log::warning() << "irccdctl: " << it->toString() << std::endl;
}

} // !irccd

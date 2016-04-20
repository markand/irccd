/*
 * options.cpp -- parse Unix command line options
 *
 * Copyright (c) 2015 David Demelier <markand@malikania.fr>
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

#include "options.hpp"

namespace irccd {

namespace parser {

namespace {

using Iterator = std::vector<std::string>::iterator;
using Args = std::vector<std::string>;

inline bool isOption(const std::string &arg) noexcept
{
	return arg.size() >= 2 && arg[0] == '-';
}

inline bool isLongOption(const std::string &arg) noexcept
{
	assert(isOption(arg));

	return arg.size() >= 3 && arg[1] == '-';
}

inline bool isShortSimple(const std::string &arg) noexcept
{
	assert(isOption(arg));
	assert(!isLongOption(arg));

	return arg.size() == 2;
}

void parseLongOption(Result &result, Args &args, Iterator &it, Iterator &end, const Options &definition)
{
	auto arg = *it++;
	auto opt = definition.find(arg);

	if (opt == definition.end())
		throw InvalidOption(arg);

	/* Need argument? */
	if (opt->second) {
		if (it == end || isOption(*it))
			throw MissingValue(arg);

		result.insert(std::make_pair(arg, *it++));
		it = args.erase(args.begin(), it);
		end = args.end();
	} else {
		result.insert(std::make_pair(arg, ""));
		it = args.erase(args.begin());
		end = args.end();
	}
}

void parseShortOption(Result &result, Args &args, Iterator &it, Iterator &end, const Options &definition)
{
	if (isShortSimple(*it)) {
		/*
		 * Here two cases:
		 *
		 * -v (no option)
		 * -c value
		 */
		auto arg = *it++;
		auto opt = definition.find(arg);

		if (opt == definition.end())
			throw InvalidOption(arg);

		/* Need argument? */
		if (opt->second) {
			if (it == end || isOption(*it))
				throw MissingValue(arg);

			result.insert(std::make_pair(arg, *it++));
			it = args.erase(args.begin(), it);
			end = args.end();
		} else {
			result.insert(std::make_pair(arg, ""));
			it = args.erase(args.begin());
			end = args.end();
		}
	} else {
		/*
		 * Here multiple scenarios:
		 *
		 * 1. -abc (-a -b -c if all are simple boolean arguments)
		 * 2. -vc foo.conf (-v -c foo.conf if -c is argument dependant)
		 * 3. -vcfoo.conf (-v -c foo.conf also)
		 */
		auto value = it->substr(1);
		auto len = value.length();
		int toremove = 1;

		for (decltype(len) i = 0; i < len; ++i) {
			auto arg = std::string{'-'} + value[i];
			auto opt = definition.find(arg);

			if (opt == definition.end())
				throw InvalidOption(arg);

			if (opt->second) {
				if (i == (len - 1)) {
					/* End of string, get the next argument (see 2.) */
					if (++it == end || isOption(*it))
						throw MissingValue(arg);

					result.insert(std::make_pair(arg, *it));
					toremove += 1;
				} else {
					result.insert(std::make_pair(arg, value.substr(i + 1)));
					i = len;
				}
			} else {
				result.insert(std::make_pair(arg, ""));
			}
		}

		it = args.erase(args.begin(), args.begin() + toremove);
		end = args.end();
	}
}

} // !namespace

Result read(std::vector<std::string> &args, const Options &definition)
{
	Result result;

	auto it = args.begin();
	auto end = args.end();

	while (it != end) {
		if (!isOption(*it))
			break;

		if (isLongOption(*it))
			parseLongOption(result, args, it, end, definition);
		else
			parseShortOption(result, args, it, end, definition);
	}

	return result;
}

Result read(int &argc, char **&argv, const Options &definition)
{
	std::vector<std::string> args;

	for (int i = 0; i < argc; ++i)
		args.push_back(argv[i]);

	auto before = args.size();
	auto result = read(args, definition);

	argc -= before - args.size();
	argv += before - args.size();

	return result;
}

} // !parser

} // !irccd

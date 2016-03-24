/*
 * util.cpp -- some utilities
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

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <irccd-config.h>

#include "util.h"
#include "unicode.h"

using namespace std::string_literals;

namespace irccd {

namespace util {

namespace {

const std::unordered_map<std::string, int> colorTable{
	{ "white",	0	},
	{ "black",	1	},
	{ "blue",	2	},
	{ "green",	3	},
	{ "red",	4	},
	{ "brown",	5	},
	{ "purple",	6	},
	{ "orange",	7	},
	{ "yellow",	8	},
	{ "lightgreen",	9	},
	{ "cyan",	10	},
	{ "lightcyan",	11	},
	{ "lightblue",	12	},
	{ "pink",	13	},
	{ "grey",	14	},
	{ "lightgrey",	15	}
};

const std::unordered_map<std::string, char> attributesTable{
	{ "bold",	'\x02'	},
	{ "italic",	'\x09'	},
	{ "strike",	'\x13'	},
	{ "reset",	'\x0f'	},
	{ "underline",	'\x15'	},
	{ "underline2",	'\x1f'	},
	{ "reverse",	'\x16'	}
};

std::string substituteDate(const std::string &text, const Substitution &params)
{
	std::ostringstream oss;

#if defined(HAVE_STD_PUT_TIME)
	oss << std::put_time(std::localtime(&params.time), text.c_str());
#else
	/*
	 * Quick and dirty hack because GCC does not have this function.
	 */
	char buffer[4096];

	std::strftime(buffer, sizeof (buffer) - 1, text.c_str(), std::localtime(&params.time));

	oss << buffer;
#endif

	return oss.str();
}

std::string substituteKeywords(const std::string &content, const Substitution &params)
{
	auto value = params.keywords.find(content);

	if (value != params.keywords.end())
		return value->second;

	return "";
}

std::string substituteEnv(const std::string &content)
{
	auto value = std::getenv(content.c_str());

	if (value != nullptr)
		return value;

	return "";
}

std::string substituteAttributes(const std::string &content)
{
	std::stringstream oss;
	std::vector<std::string> list = split(content, ",");

	/* @{} means reset */
	if (list.empty()) {
		oss << attributesTable.at("reset");
	} else {
		/* Remove useless spaces */
		for (auto &a : list)
			a = strip(a);

		/*
		 * 0: foreground
		 * 1: background
		 * 2-n: attributes
		 */
		auto foreground = list[0];
		if (!foreground.empty() || list.size() >= 2) {
			/* Color sequence */
			oss << '\x03';

			/* Foreground */
			auto it = colorTable.find(foreground);
			if (it != colorTable.end())
				oss << it->second;

			/* Background */
			if (list.size() >= 2 && (it = colorTable.find(list[1])) != colorTable.end())
				oss << "," << it->second;

			/* Attributes */
			for (std::size_t i = 2; i < list.size(); ++i) {
				auto attribute = attributesTable.find(list[i]);

				if (attribute != attributesTable.end())
					oss << attribute->second;
			}
		}
	}

	return oss.str();
}

std::string substitute(std::string::const_iterator &it, std::string::const_iterator &end, char token, const Substitution &params)
{
	std::string content, value;

	if (it == end)
		return "";

	while (it != end && *it != '}')
		content += *it++;

	if (*it != '}' || it == end)
		throw std::invalid_argument("unclosed "s + token + " construct"s);

	it++;

	switch (token) {
	case '#':
		value = substituteKeywords(content, params);
		break;
	case '$':
		value = substituteEnv(content);
		break;
	case '@':
		value = substituteAttributes(content);
		break;
	default:
		throw std::invalid_argument("unknown "s + token + " construct");
	}

	return value;
}

} // !namespace

std::string format(std::string text, const Substitution &params)
{
	std::ostringstream oss;

	/*
	 * Change the date format before anything else to avoid interpolation with keywords and
	 * user input.
	 */
	text = substituteDate(text, params);

	std::string::const_iterator it = text.begin();
	std::string::const_iterator end = text.end();

	while (it != end) {
		auto token = *it;

		if (token == '#' || token == '@' || token == '$') {
			++ it;

			if (it == end) {
				oss << token;
				continue;
			}

			if (*it == '{') {
				/* Do we have a variable? */
				oss << substitute(++it, end, token, params);
			} else if (*it == token) {
				/* Need one for sure */
				oss << token;

				/* Do we have a double token followed by a { for escaping? */
				if (++it == end)
					continue;

				if (*it != '{')
					oss << token;
			} else {
				oss << *it++;
			}
		} else {
			oss << *it++;
		}
	}

	return oss.str();
}

std::string strip(std::string str)
{
	auto test = [] (char c) { return !std::isspace(c); };

	str.erase(str.begin(), std::find_if(str.begin(), str.end(), test));
	str.erase(std::find_if(str.rbegin(), str.rend(), test).base(), str.end());

	return str;
}

std::vector<std::string> split(const std::string &list, const std::string &delimiters, int max)
{
	std::vector<std::string> result;
	size_t next = -1, current;
	int count = 1;
	bool finished = false;

	if (list.empty())
		return result;

	do {
		std::string val;

		current = next + 1;
		next = list.find_first_of(delimiters, current);

		// split max, get until the end
		if (max >= 0 && count++ >= max) {
			val = list.substr(current, std::string::npos);
			finished = true;
		} else {
			val = list.substr(current, next - current);
			finished = next == std::string::npos;
		}

		result.push_back(val);
	} while (!finished);

	return result;
}

MessagePair parseMessage(std::string message, const std::string &cc, const std::string &name)
{
	std::string result = message;
	bool iscommand = false;

	// handle special commands "!<plugin> command"
	if (cc.length() > 0) {
		auto pos = result.find_first_of(" \t");
		auto fullcommand = cc + name;

		/*
		 * If the message that comes is "!foo" without spaces we
		 * compare the command char + the plugin name. If there
		 * is a space, we check until we find a space, if not
		 * typing "!foo123123" will trigger foo plugin.
		 */
		if (pos == std::string::npos)
			iscommand = result == fullcommand;
		else
			iscommand = result.length() >= fullcommand.length() && result.compare(0, pos, fullcommand) == 0;

		if (iscommand) {
			/*
			 * If no space is found we just set the message to "" otherwise
			 * the plugin name will be passed through onCommand
			 */
			if (pos == std::string::npos)
				result = "";
			else
				result = message.substr(pos + 1);
		}
	}

	return MessagePair(result, ((iscommand) ? MessageType::Command : MessageType::Message));
}

bool isBoolean(std::string value) noexcept
{
	return (value = unicode::toupper(value)) == "1" || value == "YES" || value == "TRUE" || value == "ON";
}

bool isInt(const std::string &str, int base) noexcept
{
	if (str.empty())
		return false;
	
	char *ptr;
	
	std::strtol(str.c_str(), &ptr, base);
	
	return *ptr == 0;
}

bool isReal(const std::string &str) noexcept
{
	if (str.empty())
		return false;
	
	char *ptr;
	
	std::strtod(str.c_str(), &ptr);
	
	return *ptr == 0;
}

std::string nextNetwork(std::string &input)
{
	std::string result;
	std::string::size_type pos = input.find("\r\n\r\n");

	if ((pos = input.find("\r\n\r\n")) != std::string::npos) {
		result = input.substr(0, pos);
		input.erase(input.begin(), input.begin() + pos + 4);
	}

	return result;
}

} // util

} // !irccd

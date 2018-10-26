/*
 * ini.cpp -- extended .ini file parser
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
#include <cctype>
#include <cstring>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <stdexcept>

// for PathIsRelative.
#if defined(_WIN32)
#  include <Shlwapi.h>
#endif

#include "ini.hpp"

using namespace std::string_literals;

namespace irccd::ini {

namespace {

using stream_iterator = std::istreambuf_iterator<char>;
using token_iterator = std::vector<token>::const_iterator;

auto is_absolute(const std::string& path) noexcept -> bool
{
#if defined(_WIN32)
	return !PathIsRelative(path.c_str());
#else
	return path.size() > 0 && path[0] == '/';
#endif
}

auto is_quote(char c) noexcept -> bool
{
	return c == '\'' || c == '"';
}

auto is_space(char c) noexcept -> bool
{
	// Custom version because std::isspace includes \n as space.
	return c == ' ' || c == '\t';
}

auto is_list(char c) noexcept -> bool
{
	return c == '(' || c == ')' || c == ',';
}

auto is_reserved(char c) noexcept -> bool
{
	return is_list(c) || is_quote(c) || c == '[' || c == ']' || c == '@' || c == '#' || c == '=';
}

void analyse_line(unsigned& line, unsigned& column, stream_iterator& it) noexcept
{
	assert(*it == '\n');

	++ line;
	++ it;
	column = 0;
}

void analyse_comment(unsigned& column, stream_iterator& it, stream_iterator end) noexcept
{
	assert(*it == '#');

	while (it != end && *it != '\n') {
		++ column;
		++ it;
	}
}

void analyse_spaces(unsigned& column, stream_iterator& it, stream_iterator end) noexcept
{
	assert(is_space(*it));

	while (it != end && is_space(*it)) {
		++ column;
		++ it;
	}
}

void analyse_list(tokens& list, unsigned line, unsigned& column, stream_iterator& it) noexcept
{
	assert(is_list(*it));

	switch (*it++) {
	case '(':
		list.emplace_back(token::list_begin, line, column++);
		break;
	case ')':
		list.emplace_back(token::list_end, line, column++);
		break;
	case ',':
		list.emplace_back(token::comma, line, column++);
		break;
	default:
		break;
	}
}

void analyse_section(tokens& list, unsigned& line, unsigned& column, stream_iterator& it, stream_iterator end)
{
	assert(*it == '[');

	std::string value;
	unsigned save = column;

	// Read section name.
	++ it;
	while (it != end && *it != ']') {
		if (*it == '\n')
			throw exception(line, column, "section not terminated, missing ']'");
		if (is_reserved(*it))
			throw exception(line, column, "section name expected after '[', got '" + std::string(1, *it) + "'");

		++ column;
		value += *it++;
	}

	if (it == end)
		throw exception(line, column, "section name expected after '[', got <EOF>");
	if (value.empty())
		throw exception(line, column, "empty section name");

	// Remove ']'.
	++ it;

	list.emplace_back(token::section, line, save, std::move(value));
}

void analyse_assign(tokens& list, unsigned& line, unsigned& column, stream_iterator& it)
{
	assert(*it == '=');

	list.push_back({ token::assign, line, column++ });
	++ it;
}

void analyse_quoted_word(tokens& list, unsigned& line, unsigned& column, stream_iterator& it, stream_iterator end)
{
	std::string value;
	unsigned save = column;
	char quote = *it++;

	while (it != end && *it != quote) {
		// TODO: escape sequence
		++ column;
		value += *it++;
	}

	if (it == end)
		throw exception(line, column, "undisclosed '" + std::string(1, quote) + "', got <EOF>");

	// Remove quote.
	++ it;

	list.push_back({ token::quoted_word, line, save, std::move(value) });
}

void analyse_word(tokens& list, unsigned& line, unsigned& column, stream_iterator& it, stream_iterator end)
{
	assert(!is_reserved(*it));

	std::string value;
	unsigned save = column;

	while (it != end && !std::isspace(*it) && !is_reserved(*it)) {
		++ column;
		value += *it++;
	}

	list.push_back({ token::word, line, save, std::move(value) });
}

void analyse_include(tokens& list, unsigned& line, unsigned& column, stream_iterator& it, stream_iterator end)
{
	assert(*it == '@');

	std::string include;
	unsigned save = column;

	// Read include.
	++ it;
	while (it != end && !is_space(*it)) {
		++ column;
		include += *it++;
	}

	if (include == "include")
		list.push_back({ token::include, line, save });
	else if (include == "tryinclude")
		list.push_back({ token::tryinclude, line, save });
	else
		throw exception(line, column, "expected include or tryinclude after '@' token");
}

void parse_option_value_simple(option& option, token_iterator& it)
{
	assert(it->get_type() == token::word || it->get_type() == token::quoted_word);

	option.push_back((it++)->get_value());
}

void parse_option_value_list(option& option, token_iterator& it, token_iterator end)
{
	assert(it->get_type() == token::list_begin);

	token_iterator save = it++;

	while (it != end && it->get_type() != token::list_end) {
		switch (it->get_type()) {
		case token::comma:
			// Previous must be a word.
			if (it[-1].get_type() != token::word && it[-1].get_type() != token::quoted_word)
				throw exception(it->get_line(), it->get_column(),
				                "unexpected comma after '"s + it[-1].get_value() + "'");

			++ it;
			break;
		case token::word:
		case token::quoted_word:
			option.push_back((it++)->get_value());
			break;
		default:
			throw exception(it->get_line(), it->get_column(), "unexpected '"s + it[-1].get_value() + "' in list construct");
			break;
		}
	}

	if (it == end)
		throw exception(save->get_line(), save->get_column(), "unterminated list construct");

	// Remove ).
	++ it;
}

void parse_option(section& sc, token_iterator& it, token_iterator end)
{
	option option(it->get_value());
	token_iterator save(it);

	// No '=' or something else?
	if (++it == end)
		throw exception(save->get_line(), save->get_column(), "expected '=' assignment, got <EOF>");
	if (it->get_type() != token::assign)
		throw exception(it->get_line(), it->get_column(), "expected '=' assignment, got " + it->get_value());

	// Empty options are allowed so just test for words.
	if (++it != end) {
		if (it->get_type() == token::word || it->get_type() == token::quoted_word)
			parse_option_value_simple(option, it);
		else if (it->get_type() == token::list_begin)
			parse_option_value_list(option, it, end);
	}

	sc.push_back(std::move(option));
}

void parse_include(document& doc, const std::string& path, token_iterator& it, token_iterator end, bool required)
{
	token_iterator save(it);

	if (++it == end)
		throw exception(save->get_line(), save->get_column(), "expected file name after '@include' statement, got <EOF>");
	if (it->get_type() != token::word && it->get_type() != token::quoted_word)
		throw exception(it->get_line(), it->get_column(),
		                "expected file name after '@include' statement, got "s + it->get_value());

	std::string value = (it++)->get_value();
	std::string file;

	if (!is_absolute(value)) {
#if defined(_WIN32)
		file = path + "\\" + value;
#else
		file = path + "/" + value;
#endif
	} else
		file = value;

	try {
		/*
		 * If required is set to true, we have @include, otherwise the non-fatal
		 * @tryinclude keyword.
		 */
		for (const auto& sc : read_file(file))
			doc.push_back(sc);
	} catch (...) {
		if (required)
			throw;
	}
}

void parse_section(document& doc, token_iterator& it, token_iterator end)
{
	section sc(it->get_value());

	// Skip [section].
	++ it;

	// Read until next section.
	while (it != end && it->get_type() != token::section) {
		if (it->get_type() != token::word)
			throw exception(it->get_line(), it->get_column(),
			                "unexpected token '"s + it->get_value() + "' in section definition");

		parse_option(sc, it, end);
	}

	doc.push_back(std::move(sc));
}

} // !namespace

exception::exception(unsigned line, unsigned column, std::string msg) noexcept
	: line_(line)
	, column_(column)
	, message_(std::move(msg))
{
}

auto exception::line() const noexcept -> unsigned
{
	return line_;
}

auto exception::column() const noexcept -> unsigned
{
	return column_;
}

auto exception::what() const noexcept -> const char*
{
	return message_.c_str();
}

token::token(type type, unsigned line, unsigned column, std::string value) noexcept
	: type_(type)
	, line_(line)
	, column_(column)
{
	switch (type) {
	case include:
		value_ = "@include";
		break;
	case tryinclude:
		value_ = "@tryinclude";
		break;
	case section:
	case word:
	case quoted_word:
		value_ = value;
		break;
	case assign:
		value_ = "=";
		break;
	case list_begin:
		value_ = "(";
		break;
	case list_end:
		value_ = ")";
		break;
	case comma:
		value_ = ",";
		break;
	default:
		break;
	}
}

auto token::get_type() const noexcept -> type
{
	return type_;
}

auto token::get_line() const noexcept -> unsigned
{
	return line_;
}

auto token::get_column() const noexcept -> unsigned
{
	return column_;
}

auto token::get_value() const noexcept -> const std::string&
{
	return value_;
}

option::option(std::string key) noexcept
	: std::vector<std::string>()
	, key_(std::move(key))
{
	assert(!key_.empty());
}

option::option(std::string key, std::string value) noexcept
	: key_(std::move(key))
{
	assert(!key_.empty());

	push_back(std::move(value));
}

option::option(std::string key, std::vector<std::string> values) noexcept
	: std::vector<std::string>(std::move(values))
	, key_(std::move(key))
{
	assert(!key_.empty());
}

auto option::get_key() const noexcept -> const std::string&
{
	return key_;
}

auto option::get_value() const noexcept -> const std::string&
{
	static std::string dummy;

	return empty() ? dummy : (*this)[0];
}

section::section(std::string key) noexcept
	: key_(std::move(key))
{
	assert(!key_.empty());
}

auto section::get_key() const noexcept -> const std::string&
{
	return key_;
}

auto section::contains(std::string_view key) const noexcept -> bool
{
	return find(key) != end();
}

auto section::get(std::string_view key) const noexcept -> option
{
	auto it = find(key);

	if (it == end())
		return option(std::string(key));

	return *it;
}

auto section::find(std::string_view key) noexcept -> iterator
{
	return std::find_if(begin(), end(), [&] (const auto& o) {
		return o.get_key() == key;
	});
}

auto section::find(std::string_view key) const noexcept -> const_iterator
{
	return std::find_if(cbegin(), cend(), [&] (const auto& o) {
		return o.get_key() == key;
	});
}

auto section::operator[](std::string_view key) -> option&
{
	assert(contains(key));

	return *find(key);
}

auto section::operator[](std::string_view key) const -> const option&
{
	assert(contains(key));

	return *find(key);
}

auto document::contains(std::string_view key) const noexcept -> bool
{
	return find(key) != end();
}

auto document::get(std::string_view key) const noexcept -> section
{
	auto it = find(key);

	if (it == end())
		return section(std::string(key));

	return *it;
}

auto document::find(std::string_view key) noexcept -> iterator
{
	return std::find_if(begin(), end(), [&] (const auto& o) {
		return o.get_key() == key;
	});
}

auto document::find(std::string_view key) const noexcept -> const_iterator
{
	return std::find_if(cbegin(), cend(), [&] (const auto& o) {
		return o.get_key() == key;
	});
}

auto document::operator[](std::string_view key) -> section&
{
	assert(contains(key));

	return *find(key);
}

auto document::operator[](std::string_view key) const -> const section&
{
	assert(contains(key));

	return *find(key);
}

tokens analyse(std::istreambuf_iterator<char> it, std::istreambuf_iterator<char> end)
{
	tokens list;
	unsigned line = 1;
	unsigned column = 0;

	while (it != end) {
		if (*it == '\n')
			analyse_line(line, column, it);
		else if (*it == '#')
			analyse_comment(column, it, end);
		else if (*it == '[')
			analyse_section(list, line, column, it, end);
		else if (*it == '=')
			analyse_assign(list, line, column, it);
		else if (is_space(*it))
			analyse_spaces(column, it, end);
		else if (*it == '@')
			analyse_include(list, line, column, it, end);
		else if (is_quote(*it))
			analyse_quoted_word(list, line, column, it, end);
		else if (is_list(*it))
			analyse_list(list, line, column, it);
		else
			analyse_word(list, line, column, it, end);
	}

	return list;
}

tokens analyse(std::istream& stream)
{
	return analyse(std::istreambuf_iterator<char>(stream), {});
}

document parse(const tokens& tokens, const std::string& path)
{
	document doc;
	token_iterator it = tokens.cbegin();
	token_iterator end = tokens.cend();

	while (it != end) {
		switch (it->get_type()) {
		case token::include:
			parse_include(doc, path, it, end, true);
			break;
		case token::tryinclude:
			parse_include(doc, path, it, end, false);
			break;
		case token::section:
			parse_section(doc, it, end);
			break;
		default:
			throw exception(it->get_line(), it->get_column(),
			                "unexpected '"s + it->get_value() + "' on root document");
		}
	}

	return doc;
}

document read_file(const std::string& filename)
{
	// Get parent path.
	auto parent = filename;
	auto pos = parent.find_last_of("/\\");

	if (pos != std::string::npos)
		parent.erase(pos);
	else
		parent = ".";

	std::ifstream input(filename);

	if (!input)
		throw exception(0, 0, std::strerror(errno));

	return parse(analyse(input), parent);
}

document read_string(const std::string& buffer)
{
	std::istringstream iss(buffer);

	return parse(analyse(iss));
}

void dump(const tokens& tokens)
{
	for (const token& token: tokens) {
		// TODO: add better description
		std::cout << token.get_line() << ":" << token.get_column() << ": " << token.get_value() << std::endl;
	}
}

} // !irccd::ini

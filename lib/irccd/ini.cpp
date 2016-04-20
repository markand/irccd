/*
 * ini.cpp -- .ini file parsing
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

#include <cassert>
#include <cctype>
#include <cstring>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <stdexcept>

#if defined(_WIN32)
#  include <Shlwapi.h>	// for PathIsRelative
#endif

#include "ini.hpp"

namespace {

using namespace irccd;
using namespace irccd::ini;

using StreamIterator = std::istreambuf_iterator<char>;
using TokenIterator = std::vector<Token>::const_iterator;

inline bool isAbsolute(const std::string &path) noexcept
{
#if defined(_WIN32)
	return !PathIsRelative(path.c_str());
#else
	return path.size() > 0 && path[0] == '/';
#endif
}

inline bool isQuote(char c) noexcept
{
	return c == '\'' || c == '"';
}

inline bool isSpace(char c) noexcept
{
	/* Custom version because std::isspace includes \n as space */
	return c == ' ' || c == '\t';
}

inline bool isList(char c) noexcept
{
	return c == '(' || c == ')' || c == ',';
}

inline bool isReserved(char c) noexcept
{
	return isList(c) || isQuote(c) || c == '[' || c == ']' || c == '@' || c == '#' || c == '=';
}

void analyzeLine(int &line, int &column, StreamIterator &it) noexcept
{
	assert(*it == '\n');

	++ line;
	++ it;
	column = 0;
}

void analyzeComment(int &column, StreamIterator &it, StreamIterator end) noexcept
{
	assert(*it == '#');

	while (it != end && *it != '\n') {
		++ column;
		++ it;
	}
}

void analyzeSpaces(int &column, StreamIterator &it, StreamIterator end) noexcept
{
	assert(isSpace(*it));

	while (it != end && isSpace(*it)) {
		++ column;
		++ it;
	}
}

void analyzeList(Tokens &list, int line, int &column, StreamIterator &it) noexcept
{
	assert(isList(*it));

	switch (*it++) {
	case '(':
		list.emplace_back(Token::ListBegin, line, column++);
		break;
	case ')':
		list.emplace_back(Token::ListEnd, line, column++);
		break;
	case ',':
		list.emplace_back(Token::Comma, line, column++);
		break;
	default:
		break;
	}
}

void analyzeSection(Tokens &list, int &line, int &column, StreamIterator &it, StreamIterator end)
{
	assert(*it == '[');

	std::string value;
	int save = column;

	/* Read section name */
	++ it;
	while (it != end && *it != ']') {
		if (*it == '\n')
			throw Error(line, column, "section not terminated, missing ']'");
		if (isReserved(*it))
			throw Error(line, column, "section name expected after '[', got '" + std::string(1, *it) + "'");
		++ column;
		value += *it++;
	}

	if (it == end)
		throw Error(line, column, "section name expected after '[', got <EOF>");

	/* Remove ']' */
	++ it;

	list.emplace_back(Token::Section, line, save, std::move(value));
}

void analyzeAssign(Tokens &list, int &line, int &column, StreamIterator &it)
{
	assert(*it == '=');

	list.push_back({ Token::Assign, line, column++ });
	++ it;
}

void analyzeQuotedWord(Tokens &list, int &line, int &column, StreamIterator &it, StreamIterator end)
{
	std::string value;
	int save = column;
	char quote = *it++;

	while (it != end && *it != quote) {
		// TODO: escape sequence
		++ column;
		value += *it++;
	}

	if (it == end)
		throw Error(line, column, "undisclosed '" + std::string(1, quote) + "', got <EOF>");

	/* Remove quote */
	++ it;

	list.push_back({ Token::QuotedWord, line, save, std::move(value) });
}

void analyzeWord(Tokens &list, int &line, int &column, StreamIterator &it, StreamIterator end)
{
	assert(!isReserved(*it));

	std::string value;
	int save = column;

	while (it != end && !std::isspace(*it) && !isReserved(*it)) {
		++ column;
		value += *it++;
	}

	list.push_back({ Token::Word, line, save, std::move(value) });
}

void analyzeInclude(Tokens &list, int &line, int &column, StreamIterator &it, StreamIterator end)
{
	assert(*it == '@');

	std::string include;
	int save = column;

	/* Read include */
	++ it;
	while (it != end && !isSpace(*it)) {
		++ column;
		include += *it++;
	}

	if (include != "include")
		throw Error(line, column, "expected include after '@' token");

	list.push_back({ Token::Include, line, save });
}

Tokens analyze(StreamIterator &it, StreamIterator end)
{
	Tokens list;
	int line = 1;
	int column = 0;

	while (it != end) {
		if (*it == '\n')
			analyzeLine(line, column, it);
		else if (*it == '#')
			analyzeComment(column, it, end);
		else if (*it == '[')
			analyzeSection(list, line, column, it, end);
		else if (*it == '=')
			analyzeAssign(list, line, column, it);
		else if (isSpace(*it))
			analyzeSpaces(column, it, end);
		else if (*it == '@')
			analyzeInclude(list, line, column, it, end);
		else if (isQuote(*it))
			analyzeQuotedWord(list, line, column, it, end);
		else if (isList(*it))
			analyzeList(list, line, column, it);
		else
			analyzeWord(list, line, column, it, end);
	}

	return list;
}

void parseOptionValueSimple(Option &option, TokenIterator &it)
{
	assert(it->type() == Token::Word || it->type() == Token::QuotedWord);

	option.push_back((it++)->value());
}

void parseOptionValueList(Option &option, TokenIterator &it, TokenIterator end)
{
	assert(it->type() == Token::ListBegin);

	TokenIterator save = it++;

	while (it != end && it->type() != Token::ListEnd) {
		switch (it->type()) {
		case Token::Comma:
			/* Previous must be a word */
			if (it[-1].type() != Token::Word && it[-1].type() != Token::QuotedWord)
				throw Error(it->line(), it->column(), "unexpected comma after '" + it[-1].value() + "'");

			++ it;
			break;
		case Token::Word:
		case Token::QuotedWord:
			option.push_back((it++)->value());
			break;
		default:
			throw Error(it->line(), it->column(), "unexpected '" + it[-1].value() + "' in list construct");
			break;
		}
	}

	if (it == end)
		throw Error(save->line(), save->column(), "unterminated list construct");

	/* Remove ) */
	++ it;
}

void parseOption(Section &sc, TokenIterator &it, TokenIterator end)
{
	Option option(it->value());

	TokenIterator save = it;

	/* No '=' or something else? */
	if (++it == end)
		throw Error(save->line(), save->column(), "expected '=' assignment, got <EOF>");
	if (it->type() != Token::Assign)
		throw Error(it->line(), it->column(), "expected '=' assignment, got " + it->value());

	/* Empty options are allowed so just test for words */
	if (++it != end) {
		if (it->type() == Token::Word || it->type() == Token::QuotedWord)
			parseOptionValueSimple(option, it);
		else if (it->type() == Token::ListBegin)
			parseOptionValueList(option, it, end);
	}

	sc.push_back(std::move(option));
}

void parseInclude(Document &doc, TokenIterator &it, TokenIterator end)
{
	TokenIterator save = it;

	if (++it == end)
		throw Error(save->line(), save->column(), "expected file name after '@include' statement, got <EOF>");
	if (it->type() != Token::Word && it->type() != Token::QuotedWord)
		throw Error(it->line(), it->column(), "expected file name after '@include' statement, got " + it->value());
	if (doc.path().empty())
		throw Error(it->line(), it->column(), "'@include' statement invalid with buffer documents");

	std::string value = (it++)->value();
	std::string file;

	if (!isAbsolute(value)) {
#if defined(_WIN32)
		file = doc.path() + "\\" + value;
#else
		file = doc.path() + "/" + value;
#endif
	} else {
		file = value;
	}

	Document child(File{file});

	for (const auto &sc : child)
		doc.push_back(sc);
}

void parseSection(Document &doc, TokenIterator &it, TokenIterator end)
{
	Section sc(it->value());

	/* Skip [section] */
	++ it;

	/* Read until next section */
	while (it != end && it->type() != Token::Section) {
		if (it->type() != Token::Word)
			throw Error(it->line(), it->column(), "unexpected token '" + it->value() + "' in section definition");

		parseOption(sc, it, end);
	}

	doc.push_back(std::move(sc));
}

void parse(Document &doc, const Tokens &tokens)
{
	TokenIterator it = tokens.cbegin();
	TokenIterator end = tokens.cend();

	while (it != end) {
		/* Just ignore this */
		switch (it->type()) {
		case Token::Include:
			parseInclude(doc, it, end);
			break;
		case Token::Section:
			parseSection(doc, it, end);
			break;
		default:
			throw Error(it->line(), it->column(), "unexpected '" + it->value() + "' on root document");
		}
	}
}

} // !namespace

namespace irccd {

namespace ini {

Tokens Document::analyze(const File &file)
{
	std::fstream stream(file.path);

	if (!stream)
		throw std::runtime_error(std::strerror(errno));

	std::istreambuf_iterator<char> it(stream);
	std::istreambuf_iterator<char> end;

	return ::analyze(it, end);
}

Tokens Document::analyze(const Buffer &buffer)
{
	std::istringstream stream(buffer.text);
	std::istreambuf_iterator<char> it(stream);
	std::istreambuf_iterator<char> end;

	return ::analyze(it, end);
}

Document::Document(const File &file)
	: m_path(file.path)
{
	/* Update path */
	auto pos = m_path.find_last_of("/\\");

	if (pos != std::string::npos)
		m_path.erase(pos);
	else
		m_path = ".";

	parse(*this, analyze(file));
}

Document::Document(const Buffer &buffer)
{
	parse(*this, analyze(buffer));
}

void Document::dump(const Tokens &tokens)
{
	for (const Token &token: tokens)
		// TODO: add better description
		std::cout << token.line() << ":" << token.column() << ": " << token.value() << std::endl;
}

} // !ini

} // !irccd

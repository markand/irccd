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

#include <cctype>
#include <cstring>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <irccd/sysconfig.hpp>

// for PathIsRelative.
#if defined(IRCCD_SYSTEM_WINDOWS)
#  include <shlwapi.h>
#endif

#include "ini.hpp"

namespace irccd {

namespace ini {

namespace {

using stream_iterator = std::istreambuf_iterator<char>;
using token_iterator = std::vector<token>::const_iterator;

inline bool is_absolute(const std::string& path) noexcept
{
#if defined(IRCCD_SYSTEM_WINDOWS)
    return !PathIsRelative(path.c_str());
#else
    return path.size() > 0 && path[0] == '/';
#endif
}

inline bool is_quote(char c) noexcept
{
    return c == '\'' || c == '"';
}

inline bool is_space(char c) noexcept
{
    // Custom version because std::isspace includes \n as space.
    return c == ' ' || c == '\t';
}

inline bool is_list(char c) noexcept
{
    return c == '(' || c == ')' || c == ',';
}

inline bool is_reserved(char c) noexcept
{
    return is_list(c) || is_quote(c) || c == '[' || c == ']' || c == '@' || c == '#' || c == '=';
}

void analyse_line(int& line, int& column, stream_iterator& it) noexcept
{
    assert(*it == '\n');

    ++ line;
    ++ it;
    column = 0;
}

void analyse_comment(int& column, stream_iterator& it, stream_iterator end) noexcept
{
    assert(*it == '#');

    while (it != end && *it != '\n') {
        ++ column;
        ++ it;
    }
}

void analyse_spaces(int& column, stream_iterator& it, stream_iterator end) noexcept
{
    assert(is_space(*it));

    while (it != end && is_space(*it)) {
        ++ column;
        ++ it;
    }
}

void analyse_list(tokens& list, int line, int& column, stream_iterator& it) noexcept
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

void analyse_section(tokens& list, int& line, int& column, stream_iterator& it, stream_iterator end)
{
    assert(*it == '[');

    std::string value;
    int save = column;

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

void analyse_assign(tokens& list, int& line, int& column, stream_iterator& it)
{
    assert(*it == '=');

    list.push_back({ token::assign, line, column++ });
    ++ it;
}

void analyse_quoted_word(tokens& list, int& line, int& column, stream_iterator& it, stream_iterator end)
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
        throw exception(line, column, "undisclosed '" + std::string(1, quote) + "', got <EOF>");

    // Remove quote.
    ++ it;

    list.push_back({ token::quoted_word, line, save, std::move(value) });
}

void analyse_word(tokens& list, int& line, int& column, stream_iterator& it, stream_iterator end)
{
    assert(!is_reserved(*it));

    std::string value;
    int save = column;

    while (it != end && !std::isspace(*it) && !is_reserved(*it)) {
        ++ column;
        value += *it++;
    }

    list.push_back({ token::word, line, save, std::move(value) });
}

void analyse_include(tokens& list, int& line, int& column, stream_iterator& it, stream_iterator end)
{
    assert(*it == '@');

    std::string include;
    int save = column;

    // Read include.
    ++ it;
    while (it != end && !is_space(*it)) {
        ++ column;
        include += *it++;
    }

    if (include != "include")
        throw exception(line, column, "expected include after '@' token");

    list.push_back({ token::include, line, save });
}

void parse_option_value_simple(option& option, token_iterator& it)
{
    assert(it->type() == token::word || it->type() == token::quoted_word);

    option.push_back((it++)->value());
}

void parse_option_value_list(option& option, token_iterator& it, token_iterator end)
{
    assert(it->type() == token::list_begin);

    token_iterator save = it++;

    while (it != end && it->type() != token::list_end) {
        switch (it->type()) {
        case token::comma:
            // Previous must be a word.
            if (it[-1].type() != token::word && it[-1].type() != token::quoted_word)
                throw exception(it->line(), it->column(), "unexpected comma after '" + it[-1].value() + "'");

            ++ it;
            break;
        case token::word:
        case token::quoted_word:
            option.push_back((it++)->value());
            break;
        default:
            throw exception(it->line(), it->column(), "unexpected '" + it[-1].value() + "' in list construct");
            break;
        }
    }

    if (it == end)
        throw exception(save->line(), save->column(), "unterminated list construct");

    // Remove ).
    ++ it;
}

void parse_option(section& sc, token_iterator& it, token_iterator end)
{
    option option(it->value());
    token_iterator save(it);

    // No '=' or something else?
    if (++it == end)
        throw exception(save->line(), save->column(), "expected '=' assignment, got <EOF>");
    if (it->type() != token::assign)
        throw exception(it->line(), it->column(), "expected '=' assignment, got " + it->value());

    // Empty options are allowed so just test for words.
    if (++it != end) {
        if (it->type() == token::word || it->type() == token::quoted_word)
            parse_option_value_simple(option, it);
        else if (it->type() == token::list_begin)
            parse_option_value_list(option, it, end);
    }

    sc.push_back(std::move(option));
}

void parse_include(document& doc, const std::string& path, token_iterator& it, token_iterator end)
{
    token_iterator save(it);

    if (++it == end)
        throw exception(save->line(), save->column(), "expected file name after '@include' statement, got <EOF>");
    if (it->type() != token::word && it->type() != token::quoted_word)
        throw exception(it->line(), it->column(), "expected file name after '@include' statement, got " + it->value());

    std::string value = (it++)->value();
    std::string file;

    if (!is_absolute(value)) {
#if defined(IRCCD_SYSTEM_WINDOWS)
        file = path + "\\" + value;
#else
        file = path + "/" + value;
#endif
    } else
        file = value;

    for (const auto& sc : read_file(file))
        doc.push_back(sc);
}

void parse_section(document& doc, token_iterator& it, token_iterator end)
{
    section sc(it->value());

    // Skip [section].
    ++ it;

    // Read until next section.
    while (it != end && it->type() != token::section) {
        if (it->type() != token::word)
            throw exception(it->line(), it->column(), "unexpected token '" + it->value() + "' in section definition");

        parse_option(sc, it, end);
    }

    doc.push_back(std::move(sc));
}

} // !namespace

tokens analyse(std::istreambuf_iterator<char> it, std::istreambuf_iterator<char> end)
{
    tokens list;
    int line = 1;
    int column = 0;

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
        switch (it->type()) {
        case token::include:
            parse_include(doc, path, it, end);
            break;
        case token::section:
            parse_section(doc, it, end);
            break;
        default:
            throw exception(it->line(), it->column(), "unexpected '" + it->value() + "' on root document");
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
        std::cout << token.line() << ":" << token.column() << ": " << token.value() << std::endl;
    }
}

} // !ini

} // !irccd

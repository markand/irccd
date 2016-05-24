/*
 * ini.hpp -- extended .ini file parser
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

#ifndef IRCCD_INI_HPP
#define IRCCD_INI_HPP

/**
 * \file ini.hpp
 * \brief Extended .ini file parser.
 * \author David Demelier <markand@malikania.fr>
 */

/**
 * \page Ini Ini
 * \brief Extended .ini file parser.
 *
 *   - \subpage ini-syntax
 */

/**
 * \page ini-syntax Syntax
 * \brief File syntax.
 *
 * The syntax is similar to most of `.ini` implementations as:
 *
 *   - a section is delimited by `[name]` can be redefined multiple times,
 *   - an option **must** always be defined in a section,
 *   - empty options must be surrounded by quotes,
 *   - lists can not includes trailing commas,
 *   - include statement must always be at the beginning of files (in no sections),
 *   - comments starts with # until the end of line,
 *   - options with spaces **must** use quotes.
 *
 * # Basic file
 *
 * ````ini
 * # This is a comment.
 * [section]
 * option1 = value1
 * option2 = "value 2 with spaces"    # comment is also allowed here
 * ````
 *
 * # Redefinition
 *
 * Sections can be redefined multiple times and are kept the order they are seen.
 *
 * ````ini
 * [section]
 * value = "1"
 * 
 * [section]
 * value = "2"
 * ````
 *
 * The ini::Document object will contains two ini::Section.
 *
 * # Lists
 *
 * Lists are defined using `()` and commas, like values, they may have quotes.
 *
 * ````ini
 * [section]
 * names = ( "x1", "x2" )
 *
 * # This is also allowed
 * biglist = (
 *   "abc",
 *   "def"
 * )
 * ````
 *
 * # Include statement
 *
 * You can split a file into several pieces, if the include statement contains a relative path, the path will be relative
 * to the current file being parsed.
 *
 * You **must** use the include statement before any section.
 *
 * If the file contains spaces, use quotes.
 *
 * ````ini
 * # main.conf
 * @include "foo.conf"
 *
 * # foo.conf
 * [section]
 * option1 = value1
 * ````
 */

#include <algorithm>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "sysconfig.hpp"

namespace irccd {

/**
 * Namespace for ini related classes.
 */
namespace ini {

class Document;

/**
 * \class Error
 * \brief Error in a file.
 */
class Error : public std::exception {
private:
	int m_line;		//!< line number
	int m_column;		//!< line column
	std::string m_message;	//!< error message

public:
	/**
	 * Constructor.
	 *
	 * \param line the line
	 * \param column the column
	 * \param msg the message
	 */
	inline Error(int line, int column, std::string msg) noexcept
		: m_line(line)
		, m_column(column)
		, m_message(std::move(msg))
	{
	}

	/**
	 * Get the line number.
	 *
	 * \return the line
	 */
	inline int line() const noexcept
	{
		return m_line;
	}

	/**
	 * Get the column number.
	 *
	 * \return the column
	 */
	inline int column() const noexcept
	{
		return m_column;
	}

	/**
	 * Return the raw error message (no line and column shown).
	 *
	 * \return the error message
	 */
	const char *what() const noexcept override
	{
		return m_message.c_str();
	}
};

/**
 * \class Token
 * \brief Describe a token read in the .ini source.
 *
 * This class can be used when you want to parse a .ini file yourself.
 *
 * \see analyze
 */
class Token {
public:
	/**
	 * \brief Token type.
	 */
	enum Type {
		Include,	//!< include statement
		Section,	//!< [section]
		Word,		//!< word without quotes
		QuotedWord,	//!< word with quotes
		Assign,		//!< = assignment
		ListBegin,	//!< begin of list (
		ListEnd,	//!< end of list )
		Comma		//!< list separation
	};

private:
	Type m_type;
	int m_line;
	int m_column;
	std::string m_value;

public:
	/**
	 * Construct a token.
	 *
	 * \param type the type
	 * \param line the line
	 * \param column the column
	 * \param value the value
	 */
	Token(Type type, int line, int column, std::string value = "") noexcept
		: m_type(type)
		, m_line(line)
		, m_column(column)
	{
		switch (type) {
		case Include:
			m_value = "@include";
			break;
		case Section:
		case Word:
		case QuotedWord:
			m_value = value;
			break;
		case Assign:
			m_value = "=";
			break;
		case ListBegin:
			m_value = "(";
			break;
		case ListEnd:
			m_value = ")";
			break;
		case Comma:
			m_value = ",";
			break;
		default:
			break;
		}
	}

	/**
	 * Get the type.
	 *
	 * \return the type
	 */
	inline Type type() const noexcept
	{
		return m_type;
	}

	/**
	 * Get the line.
	 *
	 * \return the line
	 */
	inline int line() const noexcept
	{
		return m_line;
	}

	/**
	 * Get the column.
	 *
	 * \return the column
	 */
	inline int column() const noexcept
	{
		return m_column;
	}

	/**
	 * Get the value. For words, quoted words and section, the value is the content. Otherwise it's the
	 * characters parsed.
	 *
	 * \return the value
	 */
	inline const std::string &value() const noexcept
	{
		return m_value;
	}
};

/**
 * List of tokens in order they are analyzed.
 */
using Tokens = std::vector<Token>;

/**
 * \class Option
 * \brief Option definition.
 */
class Option : public std::vector<std::string> {
private:
	std::string m_key;

public:
	/**
	 * Construct an empty option.
	 *
	 * \pre key must not be empty
	 * \param key the key
	 */
	inline Option(std::string key) noexcept
		: std::vector<std::string>()
		, m_key(std::move(key))
	{
		assert(!m_key.empty());
	}

	/**
	 * Construct a single option.
	 *
	 * \pre key must not be empty
	 * \param key the key
	 * \param value the value
	 */
	inline Option(std::string key, std::string value) noexcept
		: m_key(std::move(key))
	{
		assert(!m_key.empty());

		push_back(std::move(value));
	}

	/**
	 * Construct a list option.
	 *
	 * \pre key must not be empty
	 * \param key the key
	 * \param values the values
	 */
	inline Option(std::string key, std::vector<std::string> values) noexcept
		: std::vector<std::string>(std::move(values))
		, m_key(std::move(key))
	{
		assert(!m_key.empty());
	}

	/**
	 * Get the option key.
	 *
	 * \return the key
	 */
	inline const std::string &key() const noexcept
	{
		return m_key;
	}

	/**
	 * Get the option value.
	 *
	 * \return the value
	 */
	inline const std::string &value() const noexcept
	{
		static std::string dummy;

		return empty() ? dummy : (*this)[0];
	}
};

/**
 * \class Section
 * \brief Section that contains one or more options.
 */
class Section : public std::vector<Option> {
private:
	std::string m_key;

public:
	/**
	 * Construct a section with its name.
	 *
	 * \pre key must not be empty
	 * \param key the key
	 */
	inline Section(std::string key) noexcept
		: m_key(std::move(key))
	{
		assert(!m_key.empty());
	}

	/**
	 * Get the section key.
	 *
	 * \return the key
	 */
	inline const std::string &key() const noexcept
	{
		return m_key;
	}

	/**
	 * Check if the section contains a specific option.
	 *
	 * \param key the option key
	 * \return true if the option exists
	 */
	inline bool contains(const std::string &key) const noexcept
	{
		return find(key) != end();
	}

	/**
	 * Find an option by key and return an iterator.
	 *
	 * \param key the key
	 * \return the iterator or end() if not found
	 */
	inline iterator find(const std::string &key) noexcept
	{
		return std::find_if(begin(), end(), [&] (const auto &o) {
			return o.key() == key;
		});
	}

	/**
	 * Find an option by key and return an iterator.
	 *
	 * \param key the key
	 * \return the iterator or end() if not found
	 */
	inline const_iterator find(const std::string &key) const noexcept
	{
		return std::find_if(cbegin(), cend(), [&] (const auto &o) {
			return o.key() == key;
		});
	}

	/**
	 * Access an option at the specified key.
	 *
	 * \param key the key
	 * \return the option
	 * \pre contains(key) must return true
	 */
	inline Option &operator[](const std::string &key)
	{
		assert(contains(key));

		return *find(key);
	}

	/**
	 * Overloaded function.
	 *
	 * \param key the key
	 * \return the option
	 * \pre contains(key) must return true
	 */
	inline const Option &operator[](const std::string &key) const
	{
		assert(contains(key));

		return *find(key);
	}

	/**
	 * Inherited operators.
	 */
	using std::vector<Option>::operator[];
};

/**
 * \class Document
 * \brief Ini document description.
 * \see readFile
 * \see readString
 */
class Document : public std::vector<Section> {
public:
	/**
	 * Check if a document has a specific section.
	 *
	 * \param key the key
	 * \return true if the document contains the section
	 */
	inline bool contains(const std::string &key) const noexcept
	{
		return std::find_if(begin(), end(), [&] (const auto &sc) { return sc.key() == key; }) != end();
	}

	/**
	 * Find a section by key and return an iterator.
	 *
	 * \param key the key
	 * \return the iterator or end() if not found
	 */
	inline iterator find(const std::string &key) noexcept
	{
		return std::find_if(begin(), end(), [&] (const auto &o) {
			return o.key() == key;
		});
	}

	/**
	 * Find a section by key and return an iterator.
	 *
	 * \param key the key
	 * \return the iterator or end() if not found
	 */
	inline const_iterator find(const std::string &key) const noexcept
	{
		return std::find_if(cbegin(), cend(), [&] (const auto &o) {
			return o.key() == key;
		});
	}

	/**
	 * Access a section at the specified key.
	 *
	 * \param key the key
	 * \return the section
	 * \pre contains(key) must return true
	 */
	inline Section &operator[](const std::string &key)
	{
		assert(contains(key));

		return *find(key);
	}

	/**
	 * Overloaded function.
	 *
	 * \param key the key
	 * \return the section
	 * \pre contains(key) must return true
	 */
	inline const Section &operator[](const std::string &key) const
	{
		assert(contains(key));

		return *find(key);
	}

	/**
	 * Inherited operators.
	 */
	using std::vector<Section>::operator[];
};

/**
 * Analyse a stream and detect potential syntax errors. This does not parse the file like including other
 * files in include statement.
 *
 * It does only analysis, for example if an option is defined under no section, this does not trigger an
 * error while it's invalid.
 *
 * \param it the iterator
 * \param end where to stop
 * \return the list of tokens
 * \throws Error on errors
 */
IRCCD_EXPORT Tokens analyse(std::istreambuf_iterator<char> it, std::istreambuf_iterator<char> end);

/**
 * Overloaded function for stream.
 *
 * \param stream the stream
 * \return the list of tokens
 * \throws Error on errors
 */
IRCCD_EXPORT Tokens analyse(std::istream &stream);

/**
 * Parse the produced tokens.
 *
 * \param tokens the tokens
 * \param path the parent path
 * \return the document
 * \throw Error on errors
 */
IRCCD_EXPORT Document parse(const Tokens &tokens, const std::string &path = ".");

/**
 * Parse a file.
 *
 * \param filename the file name
 * \return the document
 * \throw Error on errors
 */
IRCCD_EXPORT Document readFile(const std::string &filename);

/**
 * Parse a string.
 *
 * If the string contains include statements, they are relative to the current working directory.
 *
 * \param buffer the buffer
 * \return the document
 * \throw Error on errors
 */
IRCCD_EXPORT Document readString(const std::string &buffer);

/**
 * Show all tokens and their description.
 *
 * \param tokens the tokens
 */
IRCCD_EXPORT void dump(const Tokens &tokens);

} // !ini

} // !irccd

#endif // !IRCCD_INI_HPP

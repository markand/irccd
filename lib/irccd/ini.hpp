/*
 * ini.hpp -- .ini file parsing
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
 * @file ini.hpp
 * @brief Configuration file parser.
 */

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace irccd {

/**
 * Namespace for ini related classes.
 */
namespace ini {

class Document;

/**
 * @class Error
 * @brief Error in a file
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
	 * @param l the line
	 * @param c the column
	 * @param m the message
	 */
	inline Error(int l, int c, std::string m) noexcept
		: m_line(l)
		, m_column(c)
		, m_message(std::move(m))
	{
	}

	/**
	 * Get the line number.
	 *
	 * @return the line
	 */
	inline int line() const noexcept
	{
		return m_line;
	}

	/**
	 * Get the column number.
	 *
	 * @return the column
	 */
	inline int column() const noexcept
	{
		return m_column;
	}

	/**
	 * Return the raw error message (no line and column shown).
	 *
	 * @return the error message
	 */
	const char *what() const noexcept override
	{
		return m_message.c_str();
	}
};

/**
 * @class Token
 * @brief Describe a token read in the .ini source
 *
 * This class can be used when you want to parse a .ini file yourself.
 *
 * @see Document::analyze
 */
class Token {
public:
	/**
	 * @brief Token type
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
	 * @param type the type
	 * @param line the line
	 * @param column the column
	 * @param value the value
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
	 * @return the type
	 */
	inline Type type() const noexcept
	{
		return m_type;
	}

	/**
	 * Get the line.
	 *
	 * @return the line
	 */
	inline int line() const noexcept
	{
		return m_line;
	}

	/**
	 * Get the column.
	 *
	 * @return the column
	 */
	inline int column() const noexcept
	{
		return m_column;
	}

	/**
	 * Get the value. For words, quoted words and section, the value is the content. Otherwise it's the
	 * characters parsed.
	 *
	 * @return the value
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
 * @class Option
 * @brief Option definition.
 */
class Option : public std::vector<std::string> {
private:
	std::string m_key;

public:
	/**
	 * Construct an empty option.
	 *
	 * @param key the key
	 * @param value the value
	 */
	inline Option(std::string key) noexcept
		: std::vector<std::string>()
		, m_key(std::move(key))
	{
	}

	/**
	 * Construct a single option.
	 *
	 * @param key the key
	 * @param value the value
	 */
	inline Option(std::string key, std::string value) noexcept
		: m_key(std::move(key))
	{
		push_back(std::move(value));
	}

	/**
	 * Construct a list option.
	 *
	 * @param key the key
	 * @param values the values
	 */
	inline Option(std::string key, std::vector<std::string> values) noexcept
		: std::vector<std::string>(std::move(values))
		, m_key(std::move(key))
	{
	}

	/**
	 * Get the option key.
	 *
	 * @return the key
	 */
	inline const std::string &key() const noexcept
	{
		return m_key;
	}

	/**
	 * Get the option value.
	 *
	 * @return the value
	 */
	inline const std::string &value() const noexcept
	{
		static std::string dummy;

		return empty() ? dummy : (*this)[0];
	}
};

/**
 * @class Section
 * @brief Section that contains one or more options.
 */
class Section : public std::vector<Option> {
private:
	std::string m_key;

public:
	/**
	 * Construct a section with its name.
	 *
	 * @param key the key
	 */
	inline Section(std::string key) noexcept
		: m_key(std::move(key))
	{
	}

	/**
	 * Get the section key.
	 *
	 * @return the key
	 */
	inline const std::string &key() const noexcept
	{
		return m_key;
	}

	/**
	 * Check if the section contains a specific option.
	 *
	 * @param key the option key
	 * @return true if the option exists
	 */
	inline bool contains(const std::string &key) const noexcept
	{
		return find(key) != end();
	}

	/**
	 * Access an option at the specified key.
	 *
	 * @param key the key
	 * @return the option
	 * @throw std::out_of_range if the key does not exist
	 */
	inline Option &operator[](const std::string &key)
	{
		return *find(key);
	}

	/**
	 * Access an option at the specified key.
	 *
	 * @param key the key
	 * @return the option
	 * @throw std::out_of_range if the key does not exist
	 */
	inline const Option &operator[](const std::string &key) const
	{
		return *find(key);
	}

	/**
	 * Find an option by key and return an iterator.
	 *
	 * @param key the key
	 * @return the iterator or end() if not found
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
	 * @param key the key
	 * @return the iterator or end() if not found
	 */
	inline const_iterator find(const std::string &key) const noexcept
	{
		return std::find_if(cbegin(), cend(), [&] (const auto &o) {
			return o.key() == key;
		});
	}

	/**
	 * Inherited operators.
	 */
	using std::vector<Option>::operator[];
};

/**
 * @class File
 * @brief Source for reading .ini files.
 */
class File {
public:
	/**
	 * Path to the file.
	 */
	std::string path;
};

/**
 * @class Buffer
 * @brief Source for reading ini from text.
 * @note the include statement is not supported with buffers.
 */
class Buffer {
public:
	/**
	 * The ini content.
	 */
	std::string text;
};

/**
 * @class Document
 * @brief Ini config file loader
 */
class Document : public std::vector<Section> {
private:
	std::string m_path;

public:
	/**
	 * Analyze a file and extract tokens. If the function succeeds, that does not mean the content is valid,
	 * it just means that there are no syntax error.
	 *
	 * For example, this class does not allow adding options under no sections and this function will not
	 * detect that issue.
	 *
	 * @param file the file to read
	 * @return the list of tokens
	 * @throws Error on errors
	 */
	static Tokens analyze(const File &file);

	/**
	 * Overloaded function for buffers.
	 *
	 * @param buffer the buffer to read
	 * @return the list of tokens
	 * @throws Error on errors
	 */
	static Tokens analyze(const Buffer &buffer);

	/**
	 * Show all tokens and their description.
	 *
	 * @param tokens the tokens
	 */
	static void dump(const Tokens &tokens);

	/**
	 * Construct a document from a file.
	 *
	 * @param file the file to read
	 * @throws Error on errors
	 */
	Document(const File &file);

	/**
	 * Overloaded constructor for buffers.
	 *
	 * @param buffer the buffer to read
	 * @throws Error on errors
	 */
	Document(const Buffer &buffer);

	/**
	 * Get the current document path, only useful when constructed from File source.
	 *
	 * @return the path
	 */
	inline const std::string &path() const noexcept
	{
		return m_path;
	}

	/**
	 * Check if a document has a specific section.
	 *
	 * @param key the key
	 */
	inline bool contains(const std::string &key) const noexcept
	{
		return std::find_if(begin(), end(), [&] (const auto &sc) { return sc.key() == key; }) != end();
	}

	/**
	 * Access a section at the specified key.
	 *
	 * @param key the key
	 * @return the section
	 * @throw std::out_of_range if the key does not exist
	 */
	inline Section &operator[](const std::string &key)
	{
		return *find(key);
	}

	/**
	 * Access a section at the specified key.
	 *
	 * @param key the key
	 * @return the section
	 * @throw std::out_of_range if the key does not exist
	 */
	inline const Section &operator[](const std::string &key) const
	{
		return *find(key);
	}

	/**
	 * Find a section by key and return an iterator.
	 *
	 * @param key the key
	 * @return the iterator or end() if not found
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
	 * @param key the key
	 * @return the iterator or end() if not found
	 */
	inline const_iterator find(const std::string &key) const noexcept
	{
		return std::find_if(cbegin(), cend(), [&] (const auto &o) {
			return o.key() == key;
		});
	}

	/**
	 * Inherited operators.
	 */
	using std::vector<Section>::operator[];
};

} // !ini

} // !irccd

#endif // !IRCCD_INI_HPP

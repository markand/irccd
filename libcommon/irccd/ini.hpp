/*
 * ini.hpp -- extended .ini file parser
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#ifndef INI_HPP
#define INI_HPP

/**
 * \file ini.hpp
 * \brief Extended .ini file parser.
 * \author David Demelier <markand@malikania.fr>
 * \version 1.0.0
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
 *   - include statement must always be at the beginning of files
 *     (in no sections),
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
 * Sections can be redefined multiple times and are kept the order they are
 * seen.
 *
 * ````ini
 * [section]
 * value = "1"
 *
 * [section]
 * value = "2"
 * ````
 *
 * The ini::document object will contains two ini::Section.
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
 * You can split a file into several pieces, if the include statement contains a
 * relative path, the path will be relative to the current file being parsed.
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

/**
 * \cond INI_HIDDEN_SYMBOLS
 */

#if !defined(IRCCD_EXPORT)
#   if defined(INI_DLL)
#       if defined(_WIN32)
#           if defined(INI_BUILDING_DLL)
#               define IRCCD_EXPORT __declspec(dllexport)
#           else
#               define IRCCD_EXPORT __declspec(dllimport)
#           endif
#       else
#           define IRCCD_EXPORT
#       endif
#   else
#       define IRCCD_EXPORT
#   endif
#endif

/**
 * \endcond
 */

#include <sysconfig.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace irccd {

/**
 * Namespace for ini related classes.
 */
namespace ini {

class document;

/**
 * \brief exception in a file.
 */
class exception : public std::exception {
private:
    int m_line;                 //!< line number
    int m_column;               //!< line column
    std::string m_message;      //!< exception message

public:
    /**
     * Constructor.
     *
     * \param line the line
     * \param column the column
     * \param msg the message
     */
    inline exception(int line, int column, std::string msg) noexcept
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
     * Return the raw exception message (no line and column shown).
     *
     * \return the exception message
     */
    const char* what() const noexcept override
    {
        return m_message.c_str();
    }
};

/**
 * \brief Describe a token read in the .ini source.
 *
 * This class can be used when you want to parse a .ini file yourself.
 *
 * \see analyze
 */
class token {
public:
    /**
     * \brief token type.
     */
    enum type {
        include,                //!< include statement
        section,                //!< [section]
        word,                   //!< word without quotes
        quoted_word,            //!< word with quotes
        assign,                 //!< = assignment
        list_begin,             //!< begin of list (
        list_end,               //!< end of list )
        comma                   //!< list separation
    };

private:
    type m_type;
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
    token(type type, int line, int column, std::string value = "") noexcept
        : m_type(type)
        , m_line(line)
        , m_column(column)
    {
        switch (type) {
        case include:
            m_value = "@include";
            break;
        case section:
        case word:
        case quoted_word:
            m_value = value;
            break;
        case assign:
            m_value = "=";
            break;
        case list_begin:
            m_value = "(";
            break;
        case list_end:
            m_value = ")";
            break;
        case comma:
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
    inline type type() const noexcept
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
     * Get the value. For words, quoted words and section, the value is the
     * content. Otherwise it's the characters parsed.
     *
     * \return the value
     */
    inline const std::string& value() const noexcept
    {
        return m_value;
    }
};

/**
 * List of tokens in order they are analyzed.
 */
using tokens = std::vector<token>;

/**
 * \brief option definition.
 */
class option : public std::vector<std::string> {
private:
    std::string m_key;

public:
    /**
     * Construct an empty option.
     *
     * \pre key must not be empty
     * \param key the key
     */
    inline option(std::string key) noexcept
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
    inline option(std::string key, std::string value) noexcept
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
    inline option(std::string key, std::vector<std::string> values) noexcept
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
    inline const std::string& key() const noexcept
    {
        return m_key;
    }

    /**
     * Get the option value.
     *
     * \return the value
     */
    inline const std::string& value() const noexcept
    {
        static std::string dummy;

        return empty() ? dummy : (*this)[0];
    }
};

/**
 * \brief Section that contains one or more options.
 */
class section : public std::vector<option> {
private:
    std::string m_key;

public:
    /**
     * Construct a section with its name.
     *
     * \pre key must not be empty
     * \param key the key
     */
    inline section(std::string key) noexcept
        : m_key(std::move(key))
    {
        assert(!m_key.empty());
    }

    /**
     * Get the section key.
     *
     * \return the key
     */
    inline const std::string& key() const noexcept
    {
        return m_key;
    }

    /**
     * Check if the section contains a specific option.
     *
     * \param key the option key
     * \return true if the option exists
     */
    inline bool contains(const std::string& key) const noexcept
    {
        return find(key) != end();
    }

    /**
     * Find an option by key and return an iterator.
     *
     * \param key the key
     * \return the iterator or end() if not found
     */
    inline iterator find(const std::string& key) noexcept
    {
        return std::find_if(begin(), end(), [&] (const auto& o) {
            return o.key() == key;
        });
    }

    /**
     * Find an option by key and return an iterator.
     *
     * \param key the key
     * \return the iterator or end() if not found
     */
    inline const_iterator find(const std::string& key) const noexcept
    {
        return std::find_if(cbegin(), cend(), [&] (const auto& o) {
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
    inline option& operator[](const std::string& key)
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
    inline const option& operator[](const std::string& key) const
    {
        assert(contains(key));

        return *find(key);
    }

    /**
     * Inherited operators.
     */
    using std::vector<option>::operator[];
};

/**
 * \brief Ini document description.
 * \see readFile
 * \see readString
 */
class document : public std::vector<section> {
public:
    /**
     * Check if a document has a specific section.
     *
     * \param key the key
     * \return true if the document contains the section
     */
    inline bool contains(const std::string& key) const noexcept
    {
        return find(key) != end();
    }

    /**
     * Find a section by key and return an iterator.
     *
     * \param key the key
     * \return the iterator or end() if not found
     */
    inline iterator find(const std::string& key) noexcept
    {
        return std::find_if(begin(), end(), [&] (const auto& o) {
            return o.key() == key;
        });
    }

    /**
     * Find a section by key and return an iterator.
     *
     * \param key the key
     * \return the iterator or end() if not found
     */
    inline const_iterator find(const std::string& key) const noexcept
    {
        return std::find_if(cbegin(), cend(), [&] (const auto& o) {
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
    inline section& operator[](const std::string& key)
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
    inline const section& operator[](const std::string& key) const
    {
        assert(contains(key));

        return *find(key);
    }

    /**
     * Inherited operators.
     */
    using std::vector<section>::operator[];
};

/**
 * Analyse a stream and detect potential syntax errors. This does not parse the
 * file like including other files in include statement.
 *
 * It does only analysis, for example if an option is defined under no section,
 * this does not trigger an exception while it's invalid.
 *
 * \param it the iterator
 * \param end where to stop
 * \return the list of tokens
 * \throws exception on errors
 */
IRCCD_EXPORT tokens analyse(std::istreambuf_iterator<char> it, std::istreambuf_iterator<char> end);

/**
 * Overloaded function for stream.
 *
 * \param stream the stream
 * \return the list of tokens
 * \throws exception on errors
 */
IRCCD_EXPORT tokens analyse(std::istream& stream);

/**
 * Parse the produced tokens.
 *
 * \param tokens the tokens
 * \param path the parent path
 * \return the document
 * \throw exception on errors
 */
IRCCD_EXPORT document parse(const tokens& tokens, const std::string& path = ".");

/**
 * Parse a file.
 *
 * \param filename the file name
 * \return the document
 * \throw exception on errors
 */
IRCCD_EXPORT document read_file(const std::string& filename);

/**
 * Parse a string.
 *
 * If the string contains include statements, they are relative to the current
 * working directory.
 *
 * \param buffer the buffer
 * \return the document
 * \throw exception on exceptions
 */
IRCCD_EXPORT document read_string(const std::string& buffer);

/**
 * Show all tokens and their description.
 *
 * \param tokens the tokens
 */
IRCCD_EXPORT void dump(const tokens& tokens);

} // !ini

} // !irccd

#endif // !INI_HPP

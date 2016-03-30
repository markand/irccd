/*
 * js-file.h -- Irccd.File API
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

#ifndef IRCCD_JS_FILE_H
#define IRCCD_JS_FILE_H

#include <cstdio>

#include "js.h"

namespace irccd {

/**
 * @class File
 * @brief Object for Javascript to perform I/O.
 *
 * This class can be constructed to Javascript.
 *
 * It is used in:
 *
 * - Irccd.File [constructor]
 * - Irccd.System.popen (optional)
 */
class File {
private:
	File(const File &) = delete;
	File &operator=(const File &) = delete;

	File(File &&) = delete;
	File &operator=(File &&) = delete;

protected:
	std::string m_path;
	std::FILE *m_stream;
	std::function<void (std::FILE *)> m_destructor;

public:
	/**
	 * Construct a file specified by path
	 *
	 * @param path the path
	 * @param mode the mode string (for std::fopen)
	 * @throw std::runtime_error on failures
	 */
	File(std::string path, const std::string &mode);

	/**
	 * Construct a file from a already created FILE pointer (e.g. popen).
	 *
	 * The class takes ownership of fp and will close it.
	 *
	 * @pre destructor must not be null
	 * @param fp the file pointer
	 */
	File(std::FILE *fp, std::function<void (std::FILE *)> destructor) noexcept;

	/**
	 * Closes the file.
	 */
	virtual ~File() noexcept;

	/**
	 * Get the path.
	 *
	 * @return the path
	 * @warning empty when constructed from the FILE constructor
	 */
	inline const std::string &path() const noexcept
	{
		return m_path;
	}

	inline std::FILE *handle() noexcept
	{
		return m_stream;
	}

	/**
	 * Force close, can be safely called multiple times.
	 */
	void close() noexcept;

	/**
	 * Tells if the file was closed.
	 *
	 * @return true if closed
	 */
	bool isClosed() noexcept;

	/**
	 * std::fseek wrapper.
	 *
	 * @param offset the offset
	 * @param origin the origin (SEEK_SET, *)
	 * @throw std::runtime_error on failure
	 */
	void seek(long offset, long origin);

	/**
	 * std::ftell wrapper.
	 *
	 * @return the position
	 * @throw std::runtime_error on failure
	 */
	unsigned tell();

	/**
	 * Read until the next line and discards the \\n character.
	 *
	 * @return the next line or empty if EOF
	 * @throw std::runtime_error on failure
	 */
	std::string readline();

	/**
	 * Read the specified amount of characters.
	 *
	 * If amount is less than 0, the maximum is read.
	 *
	 * @pre amount != 0
	 * @param amount the number of characters to read
	 * @return the read string
	 * @throw std::runtime_error on failure
	 */
	std::string read(int amount = -1);

	/**
	 * Write the string to the file.
	 *
	 * @param data the data to write
	 * @throw std::runtime_error on failure
	 */
	void write(const std::string &data);

	/**
	 * Check if the file reached the end.
	 *
	 * @return true if eof
	 */
	bool eof() const noexcept;
};

namespace duk {

template <>
class TypeTraits<File> {
public:
	static inline void prototype(ContextPtr ctx)
	{
		getGlobal<void>(ctx, "Irccd");
		getGlobal<void>(ctx, "File");
		getProperty<void>(ctx, -1, "prototype");
		remove(ctx, -2);
		remove(ctx, -2);
	}

	static inline std::string name()
	{
		return "\xff""\xff""File";
	}

	static inline std::vector<std::string> inherits()
	{
		return {};
	}
};

} // !duk

void loadJsFile(duk::ContextPtr ctx);

} // !irccd

#endif // !IRCCD_JS_FILE_H


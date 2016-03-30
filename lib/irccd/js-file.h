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

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>

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
	inline File(std::string path, const std::string &mode)
		: m_path(std::move(path))
		, m_destructor([] (std::FILE *fp) { std::fclose(fp); })
	{
		if ((m_stream = std::fopen(m_path.c_str(), mode.c_str())) == nullptr)
			throw std::runtime_error(std::strerror(errno));
	}

	/**
	 * Construct a file from a already created FILE pointer (e.g. popen).
	 *
	 * The class takes ownership of fp and will close it.
	 *
	 * @pre destructor must not be null
	 * @param fp the file pointer
	 */
	inline File(std::FILE *fp, std::function<void (std::FILE *)> destructor) noexcept
		: m_stream(fp)
		, m_destructor(std::move(destructor))
	{
		assert(m_destructor != nullptr);
	}

	/**
	 * Closes the file.
	 */
	virtual ~File() noexcept
	{
		close();
	}

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

	/**
	 * Get the handle.
	 *
	 * @return the handle or nullptr if the stream was closed
	 */
	inline std::FILE *handle() noexcept
	{
		return m_stream;
	}

	/**
	 * Force close, can be safely called multiple times.
	 */
	inline void close() noexcept
	{
		if (m_stream) {
			m_destructor(m_stream);
			m_stream = nullptr;
		}
	}
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


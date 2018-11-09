/*
 * file_js_api.hpp -- Irccd.File API
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

#ifndef IRCCD_JS_FILE_JS_API_HPP
#define IRCCD_JS_FILE_JS_API_HPP

/**
 * \file file_js_api.hpp
 * \brief Irccd.File Javascript API.
 */

#include <irccd/sysconfig.hpp>

#if defined(IRCCD_HAVE_STAT)
#   include <sys/types.h>
#   include <sys/stat.h>
#endif

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>

#include "js_api.hpp"

namespace irccd::js {

/**
 * \brief Object for Javascript to perform I/O.
 *
 * This class can be constructed to Javascript.
 *
 * It is used in:
 *
 * - Irccd.File [constructor]
 * - Irccd.System.popen (optional)
 */
class file {
private:
	file(const file&) = delete;
	file& operator=(const file&) = delete;

	file(file&&) = delete;
	file& operator=(file&&) = delete;

private:
	std::string path_;
	std::FILE* stream_;
	std::function<void (std::FILE*)> destructor_;

public:
	/**
	 * Construct a file specified by path
	 *
	 * \param path the path
	 * \param mode the mode string (for std::fopen)
	 * \throw std::runtime_error on failures
	 */
	inline file(std::string path, const std::string& mode)
		: path_(std::move(path))
		, destructor_([] (std::FILE* fp) { std::fclose(fp); })
	{
		if ((stream_ = std::fopen(path_.c_str(), mode.c_str())) == nullptr)
			throw std::runtime_error(std::strerror(errno));
	}

	/**
	 * Construct a file from a already created FILE pointer (e.g. popen).
	 *
	 * The class takes ownership of fp and will close it.
	 *
	 * \pre destructor must not be null
	 * \param fp the file pointer
	 * \param destructor the function to close fp (e.g. std::fclose)
	 */
	inline file(std::FILE* fp, std::function<void (std::FILE*)> destructor) noexcept
		: stream_(fp)
		, destructor_(std::move(destructor))
	{
		assert(destructor_ != nullptr);
	}

	/**
	 * Closes the file.
	 */
	virtual ~file() noexcept
	{
		close();
	}

	/**
	 * Get the path.
	 *
	 * \return the path
	 * \warning empty when constructed from the FILE constructor
	 */
	inline const std::string& get_path() const noexcept
	{
		return path_;
	}

	/**
	 * Get the handle.
	 *
	 * \return the handle or nullptr if the stream was closed
	 */
	inline std::FILE* get_handle() noexcept
	{
		return stream_;
	}

	/**
	 * Force close, can be safely called multiple times.
	 */
	inline void close() noexcept
	{
		if (stream_) {
			destructor_(stream_);
			stream_ = nullptr;
		}
	}
};

/**
 * \ingroup jsapi
 * \brief Irccd.File Javascript API.
 */
class file_js_api : public js_api {
public:
	/**
	 * \copydoc js_api::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc js_api::load
	 */
	void load(irccd& irccd, std::shared_ptr<js_plugin> plugin) override;
};

namespace duk {

/**
 * \brief Specialization for generic file type as shared_ptr.
 *
 * Supports push, require.
 */
template <>
struct type_traits<std::shared_ptr<file>> {
	/**
	 * Push a file.
	 *
	 * \pre fp != nullptr
	 * \param ctx the the context
	 * \param fp the file
	 */
	static void push(duk_context* ctx, std::shared_ptr<file> fp);

	/**
	 * Require a file. Raises a JavaScript error if not a File.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the file pointer
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> std::shared_ptr<file>;
};

#if defined(IRCCD_HAVE_STAT)

/**
 * \brief Specialization for struct stat.
 *
 * Supports push.
 */
template <>
struct type_traits<struct stat> {
	/**
	 * Push the stat information to the stack as Javascript object.
	 *
	 * \param ctx the context
	 * \param st the stat structure
	 */
	static void push(duk_context* ctx, const struct stat& st);
};

#endif // !IRCCD_HAVE_STAT

} // !duk

} // !irccd::js

#endif // !IRCCD_JS_FILE_JS_API_HPP

/*
 * file_jsapi.hpp -- Irccd.File API
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

#ifndef IRCCD_JS_FILE_JSAPI_HPP
#define IRCCD_JS_FILE_JSAPI_HPP

/**
 * \file file_jsapi.hpp
 * \brief Irccd.File Javascript API.
 */

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>

#include "jsapi.hpp"

namespace irccd {

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
    inline const std::string& path() const noexcept
    {
        return path_;
    }

    /**
     * Get the handle.
     *
     * \return the handle or nullptr if the stream was closed
     */
    inline std::FILE* handle() noexcept
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
 * \brief Irccd.File Javascript API.
 * \ingroup jsapi
 */
class file_jsapi : public jsapi {
public:
    /**
     * \copydoc jsapi::name
     */
    std::string name() const override;

    /**
     * \copydoc jsapi::load
     */
    void load(irccd& irccd, std::shared_ptr<js_plugin> plugin) override;
};

/**
 * Construct the file as this.
 *
 * The object prototype takes ownership of fp and will be deleted once
 * collected.
 *
 * \pre fp != nullptr
 * \param ctx the the context
 * \param fp the file
 */
void dukx_new_file(duk_context* ctx, file* fp);

/**
 * Push a file.
 *
 * \pre fp != nullptr
 * \param ctx the the context
 * \param fp the file
 */
void dukx_push_file(duk_context* ctx, file* fp);

/**
 * Require a file. Raises a JavaScript error if not a File.
 *
 * \param ctx the context
 * \param index the index
 */
file* dukx_require_file(duk_context* ctx, duk_idx_t index);

} // !irccd

#endif // !IRCCD_JS_FILE_JSAPI_HPP

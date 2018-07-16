/*
 * file_jsapi.cpp -- Irccd.File API
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

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <vector>

#include <boost/filesystem.hpp>

#include <irccd/fs_util.hpp>

#include "file_jsapi.hpp"
#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"

namespace irccd {

namespace {

const char *signature("\xff""\xff""irccd-file-ptr");
const char *prototype("\xff""\xff""irccd-file-prototype");

// Remove trailing \r for CRLF line style.
std::string clear_crlf(std::string input) noexcept
{
    if (input.length() > 0 && input.back() == '\r')
        input.pop_back();

    return input;
}

std::system_error from_errno() noexcept
{
    return std::system_error(make_error_code(static_cast<std::errc>(errno)));
}

std::shared_ptr<file> self(duk_context* ctx)
{
    dukx_stack_assert sa(ctx);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, signature);
    auto ptr = static_cast<std::shared_ptr<file>*>(duk_to_pointer(ctx, -1));
    duk_pop_2(ctx);

    if (!ptr)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a File object");

    return *ptr;
}

template <typename Handler>
duk_ret_t wrap(duk_context* ctx, Handler handler)
{
    try {
        return handler();
    } catch (const boost::system::system_error& ex) {
        dukx_throw(ctx, ex);
    } catch (const std::system_error& ex) {
        dukx_throw(ctx, ex);
    } catch (const std::exception& ex) {
        dukx_throw(ctx, ex);
    }

    return 0;
}

// {{{ Irccd.File.prototype.basename

/*
 * Method: Irccd.File.prototype.basename()
 * --------------------------------------------------------
 *
 * Synonym of `Irccd.File.basename(path)` but with the path from the file.
 *
 * Returns:
 *   The base name.
 */
duk_ret_t File_prototype_basename(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, fs_util::base_name(self(ctx)->get_path()));
    });
}

// }}}

// {{{ Irccd.File.prototype.close

/*
 * Method: Irccd.File.prototype.close()
 * --------------------------------------------------------
 *
 * Force close of the file, automatically called when object is collected.
 */
duk_ret_t File_prototype_close(duk_context* ctx)
{
    return wrap(ctx, [&] {
        self(ctx)->close();

        return 0;
    });
}

// }}}

// {{{ Irccd.File.prototype.dirname

/*
 * Method: Irccd.File.prototype.dirname()
 * --------------------------------------------------------
 *
 * Synonym of `Irccd.File.dirname(path)` but with the path from the file.
 *
 * Returns:
 *   The directory name.
 */
duk_ret_t File_prototype_dirname(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, fs_util::dir_name(self(ctx)->get_path()));
    });
}

// }}}

// {{{ Irccd.File.prototype.lines

/*
 * Method: Irccd.File.prototype.lines()
 * --------------------------------------------------------
 *
 * Read all lines and return an array.
 *
 * Returns:
 *   An array with all lines.
 * Throws
 *   - Any exception on error.
 */
duk_ret_t File_prototype_lines(duk_context* ctx)
{
    return wrap(ctx, [&] {
        duk_push_array(ctx);

        std::FILE* fp = self(ctx)->get_handle();
        std::string buffer;
        std::array<char, 128> data;
        std::int32_t i = 0;

        while (std::fgets(&data[0], data.size(), fp) != nullptr) {
            buffer += data.data();

            const auto pos = buffer.find('\n');

            if (pos != std::string::npos) {
                dukx_push(ctx, clear_crlf(buffer.substr(0, pos)));
                duk_put_prop_index(ctx, -2, i++);

                buffer.erase(0, pos + 1);
            }
        }

        // Maybe an error in the stream.
        if (std::ferror(fp))
            throw from_errno();

        // Missing '\n' in end of file.
        if (!buffer.empty()) {
            dukx_push(ctx, clear_crlf(buffer));
            duk_put_prop_index(ctx, -2, i++);
        }

        return 1;
    });
}

// }}}

// {{{ Irccd.File.prototype.read

/*
 * Method: Irccd.File.prototype.read(amount)
 * --------------------------------------------------------
 *
 * Read the specified amount of characters or the whole file.
 *
 * Arguments:
 *   - amount, the amount of characters or -1 to read all (Optional, default: -1).
 * Returns:
 *   The string.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_prototype_read(duk_context* ctx)
{
    return wrap(ctx, [&] {
        const auto fp = self(ctx)->get_handle();
        const auto amount = duk_is_number(ctx, 0) ? duk_get_int(ctx, 0) : -1;

        if (amount == 0 || !fp)
            return 0;

        std::string data;
        std::size_t total = 0;

        if (amount < 0) {
            std::array<char, 128> buffer;
            std::size_t nread;

            while ((nread = std::fread(&buffer[0], sizeof (buffer[0]), buffer.size(), fp)) > 0) {
                if (std::ferror(fp))
                    throw from_errno();

                std::copy(buffer.begin(), buffer.begin() + nread, std::back_inserter(data));
                total += nread;
            }
        } else {
            data.resize(static_cast<std::size_t>(amount));
            total = std::fread(&data[0], sizeof (data[0]), static_cast<std::size_t>(amount), fp);

            if (std::ferror(fp))
                throw from_errno();

            data.resize(total);
        }

        return dukx_push(ctx, data);
    });
}

// }}}

// {{{ Irccd.File.prototype.readline

/*
 * Method: Irccd.File.prototype.readline()
 * --------------------------------------------------------
 *
 * Read the next line available.
 *
 * Returns:
 *   The next line or undefined if eof.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_prototype_readline(duk_context* ctx)
{
    return wrap(ctx, [&] {
        auto fp = self(ctx)->get_handle();

        if (fp == nullptr || std::feof(fp))
            return 0;

        std::string result;

        for (int ch; (ch = std::fgetc(fp)) != EOF && ch != '\n'; )
            result += (char)ch;
        if (std::ferror(fp))
            throw from_errno();

        return dukx_push(ctx, clear_crlf(result));
    });
}

// }}}

// {{{ Irccd.File.prototype.remove

/*
 * Method: Irccd.File.prototype.remove()
 * --------------------------------------------------------
 *
 * Synonym of Irccd.File.prototype.remove(path) but with the path from the file.
 *
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_prototype_remove(duk_context* ctx)
{
    return wrap(ctx, [&] {
        boost::filesystem::remove(self(ctx)->get_path());

        return 0;
    });
}

// }}}

// {{{ Irccd.File.prototype.seek

/*
 * Method: Irccd.File.prototype.seek(type, amount)
 * --------------------------------------------------------
 *
 * Sets the position in the file.
 *
 * Arguments:
 *   - type, the type of setting (File.SeekSet, File.SeekCur, File.SeekSet),
 *   - amount, the new offset.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_prototype_seek(duk_context* ctx)
{
    return wrap(ctx, [&] {
        auto fp = self(ctx)->get_handle();
        auto type = duk_require_int(ctx, 0);
        auto amount = duk_require_int(ctx, 1);

        if (fp != nullptr && std::fseek(fp, amount, type) != 0)
            throw from_errno();

        return 0;
    });
}

// }}}

// {{{ Irccd.File.prototype.stat

#if defined(HAVE_STAT)

/*
 * Method: Irccd.File.prototype.stat() [optional]
 * --------------------------------------------------------
 *
 * Synonym of File.stat(path) but with the path from the file.
 *
 * Returns:
 *   The stat information.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_prototype_stat(duk_context* ctx)
{
    return wrap(ctx, [&] {
        auto file = self(ctx);
        struct stat st;

        if (file->get_handle() == nullptr && ::stat(file->get_path().c_str(), &st) < 0)
            throw from_errno();

        dukx_push(ctx, st);

        return 1;
    });
}

#endif // !HAVE_STAT

// }}}

// {{{ Irccd.File.prototype.tell

/*
 * Method: Irccd.File.prototype.tell()
 * --------------------------------------------------------
 *
 * Get the actual position in the file.
 *
 * Returns:
 *   The position.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_prototype_tell(duk_context* ctx)
{
    return wrap(ctx, [&] {
        auto fp = self(ctx)->get_handle();
        long pos;

        if (fp == nullptr)
            return 0;

        if ((pos = std::ftell(fp)) == -1L)
            throw from_errno();

        duk_push_int(ctx, pos);

        return 1;
    });
}

// }}}

// {{{ Irccd.File.prototype.write

/*
 * Method: Irccd.File.prototype.write(data)
 * --------------------------------------------------------
 *
 * Write some characters to the file.
 *
 * Arguments:
 *   - data, the character to write.
 * Returns:
 *   The number of bytes written.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_prototype_write(duk_context* ctx)
{
    return wrap(ctx, [&] {
        auto fp = self(ctx)->get_handle();
        auto data = dukx_require<std::string>(ctx, 0);

        if (fp == nullptr)
            return 0;

        const auto nwritten = std::fwrite(data.c_str(), 1, data.length(), fp);

        if (std::ferror(fp))
            throw from_errno();

        duk_push_uint(ctx, nwritten);

        return 1;
    });
}

// }}}

// {{{ Irccd.File [constructor]

/*
 * Function: Irccd.File(path, mode) [constructor]
 * --------------------------------------------------------
 *
 * Open a file specified by path with the specified mode.
 *
 * Arguments:
 *   - path, the path to the file,
 *   - mode, the mode string.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_constructor(duk_context* ctx)
{
    return wrap(ctx, [&] {
        if (!duk_is_constructor_call(ctx))
            return 0;

        const auto path = dukx_require<std::string>(ctx, 0);
        const auto mode = dukx_require<std::string>(ctx, 1);

        duk_push_this(ctx);
        duk_push_pointer(ctx, new std::shared_ptr<file>(new file(path, mode)));
        duk_put_prop_string(ctx, -2, signature);
        duk_pop(ctx);

        return 0;
    });
}

// }}}

// {{{ Irccd.File [destructor]

/*
 * Function: Irccd.File() [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
duk_ret_t File_destructor(duk_context* ctx)
{
    duk_get_prop_string(ctx, 0, signature);
    delete static_cast<std::shared_ptr<file>*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);
    duk_del_prop_string(ctx, 0, signature);

    return 0;
}

// }}}

// {{{ Irccd.File.basename

/*
 * Function: Irccd.File.basename(path)
 * --------------------------------------------------------
 *
 * duk_ret_turn the file basename as specified in `basename(3)` C function.
 *
 * Arguments:
 *   - path, the path to the file.
 * Returns:
 *   The base name.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_basename(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, fs_util::base_name(duk_require_string(ctx, 0)));
    });
}

// }}}

// {{{ Irccd.File.dirname

/*
 * Function: Irccd.File.dirname(path)
 * --------------------------------------------------------
 *
 * duk_ret_turn the file directory name as specified in `dirname(3)` C function.
 *
 * Arguments:
 *   - path, the path to the file.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_dirname(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, fs_util::dir_name(duk_require_string(ctx, 0)));
    });
}

// }}}

// {{{ Irccd.File.exists

/*
 * Function: Irccd.File.exists(path)
 * --------------------------------------------------------
 *
 * Check if the file exists.
 *
 * Arguments:
 *   - path, the path to the file.
 * Returns:
 *   True if exists.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_exists(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, boost::filesystem::exists(duk_require_string(ctx, 0)));
    });
}

// }}}

// {{{ Irccd.File.remove

/*
 * function Irccd.File.remove(path)
 * --------------------------------------------------------
 *
 * Remove the file at the specified path.
 *
 * Arguments:
 *   - path, the path to the file.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_remove(duk_context* ctx)
{
    return wrap(ctx, [&] {
        boost::filesystem::remove(dukx_require<std::string>(ctx, 0));

        return 0;
    });
}

// }}}

// {{{ Irccd.File.stat

#if defined(HAVE_STAT)

/*
 * function Irccd.File.stat(path) [optional]
 * --------------------------------------------------------
 *
 * Get file information at the specified path.
 *
 * Arguments:
 *   - path, the path to the file.
 * Returns:
 *   The stat information.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t File_stat(duk_context* ctx)
{
    return wrap(ctx, [&] {
        struct stat st;

        if (::stat(duk_require_string(ctx, 0), &st) < 0)
            throw from_errno();

        return dukx_push(ctx, st);
    });
}

#endif // !HAVE_STAT

// }}}

const duk_function_list_entry methods[] = {
    { "basename",   File_prototype_basename,    0 },
    { "close",      File_prototype_close,       0 },
    { "dirname",    File_prototype_dirname,     0 },
    { "lines",      File_prototype_lines,       0 },
    { "read",       File_prototype_read,        1 },
    { "readline",   File_prototype_readline,    0 },
    { "remove",     File_prototype_remove,      0 },
    { "seek",       File_prototype_seek,        2 },
#if defined(HAVE_STAT)
    { "stat",       File_prototype_stat,        0 },
#endif
    { "tell",       File_prototype_tell,        0 },
    { "write",      File_prototype_write,       1 },
    { nullptr,      nullptr,                    0 }
};

const duk_function_list_entry functions[] = {
    { "basename",   File_basename,              1 },
    { "dirname",    File_dirname,               1 },
    { "exists",     File_exists,                1 },
    { "remove",     File_remove,                1 },
#if defined(HAVE_STAT)
    { "stat",       File_stat,                  1 },
#endif
    { nullptr,      nullptr,                    0 }
};

const duk_number_list_entry constants[] = {
    { "SeekCur",    SEEK_CUR },
    { "SeekEnd",    SEEK_END },
    { "SeekSet",    SEEK_SET },
    { nullptr,      0        }
};

} // !namespace

std::string file_jsapi::get_name() const
{
    return "Irccd.File";
}

void file_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    dukx_stack_assert sa(plugin->get_context());

    duk_get_global_string(plugin->get_context(), "Irccd");
    duk_push_c_function(plugin->get_context(), File_constructor, 2);
    duk_put_number_list(plugin->get_context(), -1, constants);
    duk_put_function_list(plugin->get_context(), -1, functions);
    duk_push_object(plugin->get_context());
    duk_put_function_list(plugin->get_context(), -1, methods);
    duk_push_c_function(plugin->get_context(), File_destructor, 1);
    duk_set_finalizer(plugin->get_context(), -2);
    duk_dup(plugin->get_context(), -1);
    duk_put_global_string(plugin->get_context(), prototype);
    duk_put_prop_string(plugin->get_context(), -2, "prototype");
    duk_put_prop_string(plugin->get_context(), -2, "File");
    duk_pop(plugin->get_context());
}

using file_traits = dukx_type_traits<std::shared_ptr<file>>;

void file_traits::push(duk_context* ctx, std::shared_ptr<file> fp)
{
    assert(ctx);
    assert(fp);

    dukx_stack_assert sa(ctx, 1);

    duk_push_object(ctx);
    duk_push_pointer(ctx, new std::shared_ptr<file>(std::move(fp)));
    duk_put_prop_string(ctx, -2, signature);
    duk_get_global_string(ctx, prototype);
    duk_set_prototype(ctx, -2);
}

std::shared_ptr<file> file_traits::require(duk_context* ctx, duk_idx_t index)
{
    if (!duk_is_object(ctx, index) || !duk_has_prop_string(ctx, index, signature))
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a File object");

    duk_get_prop_string(ctx, index, signature);
    const auto fp = static_cast<std::shared_ptr<file>*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    return *fp;
}

#if defined(HAVE_STAT)

void dukx_type_traits<struct stat>::push(duk_context* ctx, const struct stat& st)
{
    dukx_stack_assert sa(ctx, 1);

    duk_push_object(ctx);

#if defined(HAVE_STAT_ST_ATIME)
    duk_push_int(ctx, st.st_atime);
    duk_put_prop_string(ctx, -2, "atime");
#endif
#if defined(HAVE_STAT_ST_BLKSIZE)
    duk_push_int(ctx, st.st_blksize);
    duk_put_prop_string(ctx, -2, "blksize");
#endif
#if defined(HAVE_STAT_ST_BLOCKS)
    duk_push_int(ctx, st.st_blocks);
    duk_put_prop_string(ctx, -2, "blocks");
#endif
#if defined(HAVE_STAT_ST_CTIME)
    duk_push_int(ctx, st.st_ctime);
    duk_put_prop_string(ctx, -2, "ctime");
#endif
#if defined(HAVE_STAT_ST_DEV)
    duk_push_int(ctx, st.st_dev);
    duk_put_prop_string(ctx, -2, "dev");
#endif
#if defined(HAVE_STAT_ST_GID)
    duk_push_int(ctx, st.st_gid);
    duk_put_prop_string(ctx, -2, "gid");
#endif
#if defined(HAVE_STAT_ST_INO)
    duk_push_int(ctx, st.st_ino);
    duk_put_prop_string(ctx, -2, "ino");
#endif
#if defined(HAVE_STAT_ST_MODE)
    duk_push_int(ctx, st.st_mode);
    duk_put_prop_string(ctx, -2, "mode");
#endif
#if defined(HAVE_STAT_ST_MTIME)
    duk_push_int(ctx, st.st_mtime);
    duk_put_prop_string(ctx, -2, "mtime");
#endif
#if defined(HAVE_STAT_ST_NLINK)
    duk_push_int(ctx, st.st_nlink);
    duk_put_prop_string(ctx, -2, "nlink");
#endif
#if defined(HAVE_STAT_ST_RDEV)
    duk_push_int(ctx, st.st_rdev);
    duk_put_prop_string(ctx, -2, "rdev");
#endif
#if defined(HAVE_STAT_ST_SIZE)
    duk_push_int(ctx, st.st_size);
    duk_put_prop_string(ctx, -2, "size");
#endif
#if defined(HAVE_STAT_ST_UID)
    duk_push_int(ctx, st.st_uid);
    duk_put_prop_string(ctx, -2, "uid");
#endif
}

#endif // !HAVE_STAT

} // !irccd

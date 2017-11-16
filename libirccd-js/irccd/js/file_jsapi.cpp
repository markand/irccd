/*
 * file_jsapi.cpp -- Irccd.File API
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

#include <irccd/sysconfig.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <vector>

#include <boost/filesystem.hpp>

#if defined(HAVE_STAT)
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include <irccd/fs_util.hpp>

#include "file_jsapi.hpp"
#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"

namespace irccd {

namespace {

const char *signature("\xff""\xff""irccd-file-ptr");
const char *prototype("\xff""\xff""irccd-file-prototype");

#if defined(HAVE_STAT)

/*
 * push_stat
 * ------------------------------------------------------------------
 */

void push_stat(duk_context* ctx, const struct stat& st)
{
    StackAssert sa(ctx, 1);

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

// Remove trailing \r for CRLF line style.
inline std::string clear_crlf(std::string input)
{
    if (input.length() > 0 && input.back() == '\r')
        input.pop_back();

    return input;
}

file* self(duk_context* ctx)
{
    StackAssert sa(ctx);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, signature);
    auto ptr = static_cast<file*>(duk_to_pointer(ctx, -1));
    duk_pop_2(ctx);

    if (!ptr)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a File object");

    return ptr;
}

/*
 * File methods.
 * ------------------------------------------------------------------
 */

/*
 * Method: File.basename()
 * --------------------------------------------------------
 *
 * Synonym of `irccd.File.basename(path)` but with the path from the file.
 *
 * duk_ret_turns:
 *   The base name.
 */
duk_ret_t method_basename(duk_context* ctx)
{
    dukx_push_std_string(ctx, fs_util::base_name(self(ctx)->path()));

    return 1;
}

/*
 * Method: File.close()
 * --------------------------------------------------------
 *
 * Force close of the file, automatically called when object is collected.
 */
duk_ret_t method_close(duk_context* ctx)
{
    self(ctx)->close();

    return 0;
}

/*
 * Method: File.dirname()
 * --------------------------------------------------------
 *
 * Synonym of `irccd.File.dirname(path)` but with the path from the file.
 *
 * duk_ret_turns:
 *   The directory name.
 */
duk_ret_t method_dirname(duk_context* ctx)
{
    dukx_push_std_string(ctx, fs_util::dir_name(self(ctx)->path()));

    return 1;
}

/*
 * Method: File.lines()
 * --------------------------------------------------------
 *
 * Read all lines and return an array.
 *
 * duk_ret_turns:
 *   An array with all lines.
 * Throws
 *   - Any exception on error.
 */
duk_ret_t method_lines(duk_context* ctx)
{
    duk_push_array(ctx);

    std::FILE* fp = self(ctx)->handle();
    std::string buffer;
    std::array<char, 128> data;
    std::int32_t i = 0;

    while (std::fgets(&data[0], data.size(), fp) != nullptr) {
        buffer += data.data();

        auto pos = buffer.find('\n');

        if (pos != std::string::npos) {
            dukx_push_std_string(ctx, clear_crlf(buffer.substr(0, pos)));
            duk_put_prop_index(ctx, -2, i++);

            buffer.erase(0, pos + 1);
        }
    }

    // Maybe an error in the stream.
    if (std::ferror(fp))
        dukx_throw(ctx, system_error());

    // Missing '\n' in end of file.
    if (!buffer.empty()) {
        dukx_push_std_string(ctx, clear_crlf(buffer));
        duk_put_prop_index(ctx, -2, i++);
    }

    return 1;
}

/*
 * Method: File.read(amount)
 * --------------------------------------------------------
 *
 * Read the specified amount of characters or the whole file.
 *
 * Arguments:
 *   - amount, the amount of characters or -1 to read all (Optional, default: -1).
 * duk_ret_turns:
 *   The string.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_read(duk_context* ctx)
{
    auto file = self(ctx);
    auto amount = duk_is_number(ctx, 0) ? duk_get_int(ctx, 0) : -1;

    if (amount == 0 || file->handle() == nullptr)
        return 0;

    try {
        std::string data;
        std::size_t total = 0;

        if (amount < 0) {
            std::array<char, 128> buffer;
            std::size_t nread;

            while ((nread = std::fread(&buffer[0], sizeof (buffer[0]), buffer.size(), file->handle())) > 0) {
                if (std::ferror(file->handle()))
                    dukx_throw(ctx, system_error());

                std::copy(buffer.begin(), buffer.begin() + nread, std::back_inserter(data));
                total += nread;
            }
        } else {
            data.resize((std::size_t)amount);
            total = std::fread(&data[0], sizeof (data[0]), (std::size_t)amount, file->handle());

            if (std::ferror(file->handle()))
                dukx_throw(ctx, system_error());

            data.resize(total);
        }

        dukx_push_std_string(ctx, data);
    } catch (const std::exception&) {
        dukx_throw(ctx, system_error());
    }

    return 1;
}

/*
 * Method: File.readline()
 * --------------------------------------------------------
 *
 * Read the next line available.
 *
 * duk_ret_turns:
 *   The next line or undefined if eof.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_readline(duk_context* ctx)
{
    std::FILE* fp = self(ctx)->handle();
    std::string result;

    if (fp == nullptr || std::feof(fp))
        return 0;
    for (int ch; (ch = std::fgetc(fp)) != EOF && ch != '\n'; )
        result += (char)ch;
    if (std::ferror(fp))
        dukx_throw(ctx, system_error());

    dukx_push_std_string(ctx, clear_crlf(result));

    return 1;
}

/*
 * Method: File.remove()
 * --------------------------------------------------------
 *
 * Synonym of File.remove(path) but with the path from the file.
 *
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_remove(duk_context* ctx)
{
    if (::remove(self(ctx)->path().c_str()) < 0)
        dukx_throw(ctx, system_error());

    return 0;
}

/*
 * Method: File.seek(type, amount)
 * --------------------------------------------------------
 *
 * Sets the position in the file.
 *
 * Arguments:
 *   - type, the type of setting (File.SeekSet, File.SeekCur, File.SeekSet),
 *   - amount, the new offset.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_seek(duk_context* ctx)
{
    auto fp = self(ctx)->handle();
    auto type = duk_require_int(ctx, 0);
    auto amount = duk_require_int(ctx, 1);

    if (fp != nullptr && std::fseek(fp, amount, type) != 0)
        dukx_throw(ctx, system_error());

    return 0;
}

#if defined(HAVE_STAT)

/*
 * Method: File.stat() [optional]
 * --------------------------------------------------------
 *
 * Synonym of File.stat(path) but with the path from the file.
 *
 * duk_ret_turns:
 *   The stat information.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_stat(duk_context* ctx)
{
    auto file = self(ctx);
    struct stat st;

    if (file->handle() == nullptr && ::stat(file->path().c_str(), &st) < 0)
        dukx_throw(ctx, system_error());
    else
        push_stat(ctx, st);

    return 1;
}

#endif // !HAVE_STAT

/*
 * Method: File.tell()
 * --------------------------------------------------------
 *
 * Get the actual position in the file.
 *
 * duk_ret_turns:
 *   The position.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_tell(duk_context* ctx)
{
    auto fp = self(ctx)->handle();
    long pos;

    if (fp == nullptr)
        return 0;

    if ((pos = std::ftell(fp)) == -1L)
        dukx_throw(ctx, system_error());
    else
        duk_push_int(ctx, pos);

    return 1;
}

/*
 * Method: File.write(data)
 * --------------------------------------------------------
 *
 * Write some characters to the file.
 *
 * Arguments:
 *   - data, the character to write.
 * duk_ret_turns:
 *   The number of bytes written.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_write(duk_context* ctx)
{
    std::FILE* fp = self(ctx)->handle();
    std::string data = duk_require_string(ctx, 0);

    if (fp == nullptr)
        return 0;

    auto nwritten = std::fwrite(data.c_str(), 1, data.length(), fp);

    if (std::ferror(fp))
        dukx_throw(ctx, system_error());

    duk_push_uint(ctx, nwritten);

    return 1;
}

const duk_function_list_entry methods[] = {
    { "basename",   method_basename,    0 },
    { "close",      method_close,       0 },
    { "dirname",    method_dirname,     0 },
    { "lines",      method_lines,       0 },
    { "read",       method_read,        1 },
    { "readline",   method_readline,    0 },
    { "remove",     method_remove,      0 },
    { "seek",       method_seek,        2 },
#if defined(HAVE_STAT)
    { "stat",       method_stat,        0 },
#endif
    { "tell",       method_tell,        0 },
    { "write",      method_write,       1 },
    { nullptr,      nullptr,            0 }
};

/*
 * File "static" functions
 * ------------------------------------------------------------------
 */

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
 *   - Any exception on error.
 */
duk_ret_t constructor(duk_context* ctx)
{
    if (!duk_is_constructor_call(ctx))
        return 0;

    try {
        dukx_new_file(ctx, new file(duk_require_string(ctx, 0), duk_require_string(ctx, 1)));
    } catch (const std::exception &) {
        dukx_throw(ctx, system_error());
    }

    return 0;
}

/*
 * Function: Irccd.File() [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
duk_ret_t destructor(duk_context* ctx)
{
    duk_get_prop_string(ctx, 0, signature);
    delete static_cast<file*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);
    duk_del_prop_string(ctx, 0, signature);

    return 0;
}

/*
 * Function: Irccd.File.basename(path)
 * --------------------------------------------------------
 *
 * duk_ret_turn the file basename as specified in `basename(3)` C function.
 *
 * Arguments:
 *   - path, the path to the file.
 * duk_ret_turns:
 *   The base name.
 */
duk_ret_t function_basename(duk_context* ctx)
{
    dukx_push_std_string(ctx, fs_util::base_name(duk_require_string(ctx, 0)));

    return 1;
}

/*
 * Function: Irccd.File.dirname(path)
 * --------------------------------------------------------
 *
 * duk_ret_turn the file directory name as specified in `dirname(3)` C function.
 *
 * Arguments:
 *   - path, the path to the file.
 * duk_ret_turns:
 *   The directory name.
 */
duk_ret_t function_dirname(duk_context* ctx)
{
    dukx_push_std_string(ctx, fs_util::dir_name(duk_require_string(ctx, 0)));

    return 1;
}

/*
 * Function: Irccd.File.exists(path)
 * --------------------------------------------------------
 *
 * Check if the file exists.
 *
 * Arguments:
 *   - path, the path to the file.
 * duk_ret_turns:
 *   True if exists.
 * Throws:
 *   - Any exception if we don't have access.
 */
duk_ret_t function_exists(duk_context* ctx)
{
    try {
        duk_push_boolean(ctx, boost::filesystem::exists(duk_require_string(ctx, 0)));
    } catch (...) {
        duk_push_boolean(ctx, false);
    }

    return 1;
}

/*
 * function Irccd.File.remove(path)
 * --------------------------------------------------------
 *
 * Remove the file at the specified path.
 *
 * Arguments:
 *   - path, the path to the file.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t function_remove(duk_context* ctx)
{
    if (::remove(duk_require_string(ctx, 0)) < 0)
        dukx_throw(ctx, system_error());

    return 0;
}

#if defined(HAVE_STAT)

/*
 * function Irccd.File.stat(path) [optional]
 * --------------------------------------------------------
 *
 * Get file information at the specified path.
 *
 * Arguments:
 *   - path, the path to the file.
 * duk_ret_turns:
 *   The stat information.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t function_stat(duk_context* ctx)
{
    struct stat st;

    if (::stat(duk_require_string(ctx, 0), &st) < 0)
        dukx_throw(ctx, system_error());

    push_stat(ctx, st);

    return 1;
}

#endif // !HAVE_STAT

const duk_function_list_entry functions[] = {
    { "basename",   function_basename,  1 },
    { "dirname",    function_dirname,   1 },
    { "exists",     function_exists,    1 },
    { "remove",     function_remove,    1 },
#if defined(HAVE_STAT)
    { "stat",       function_stat,      1 },
#endif
    { nullptr,      nullptr,            0 }
};

const duk_number_list_entry constants[] = {
    { "SeekCur",    SEEK_CUR },
    { "SeekEnd",    SEEK_END },
    { "SeekSet",    SEEK_SET },
    { nullptr,      0        }
};

} // !namespace

std::string file_jsapi::name() const
{
    return "Irccd.File";
}

void file_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    StackAssert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_c_function(plugin->context(), constructor, 2);
    duk_put_number_list(plugin->context(), -1, constants);
    duk_put_function_list(plugin->context(), -1, functions);
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, methods);
    duk_push_c_function(plugin->context(), destructor, 1);
    duk_set_finalizer(plugin->context(), -2);
    duk_dup(plugin->context(), -1);
    duk_put_global_string(plugin->context(), prototype);
    duk_put_prop_string(plugin->context(), -2, "prototype");
    duk_put_prop_string(plugin->context(), -2, "File");
    duk_pop(plugin->context());
}

void dukx_new_file(duk_context* ctx, file* fp)
{
    assert(ctx);
    assert(fp);

    StackAssert sa(ctx);

    duk_push_this(ctx);
    duk_push_pointer(ctx, fp);
    duk_put_prop_string(ctx, -2, signature);
    duk_pop(ctx);
}

void dukx_push_file(duk_context* ctx, file* fp)
{
    assert(ctx);
    assert(fp);

    StackAssert sa(ctx, 1);

    duk_push_object(ctx);
    duk_push_pointer(ctx, fp);
    duk_put_prop_string(ctx, -2, signature);
    duk_get_global_string(ctx, prototype);
    duk_set_prototype(ctx, -2);
}

file* dukx_require_file(duk_context* ctx, duk_idx_t index)
{
    if (!duk_is_object(ctx, index) || !duk_has_prop_string(ctx, index, signature))
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a File object");

    duk_get_prop_string(ctx, index, signature);
    auto fp = static_cast<file*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    return fp;
}

} // !irccd

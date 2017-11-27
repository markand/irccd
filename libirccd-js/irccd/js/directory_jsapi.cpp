/*
 * directory_jsapi.cpp -- Irccd.Directory API
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

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <string>

#include <irccd/fs_util.hpp>

#include "directory_jsapi.hpp"
#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"

namespace fs = boost::filesystem;

namespace irccd {

namespace {

std::string path(duk_context *ctx)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "path");

    if (duk_get_type(ctx, -1) != DUK_TYPE_STRING)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Directory object");

    auto ret = dukx_get<std::string>(ctx, -1);

    if (ret.empty())
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "directory object has empty path");

    duk_pop_n(ctx, 2);

    return ret;
}

/*
 * Generic find function for:
 *
 * - Directory.find
 * - Directory.prototype.find
 *
 * The patternIndex is the argument where to test if the argument is a regex or
 * a string.
 */
duk_ret_t find(duk_context* ctx, std::string base, bool recursive, int pattern_index)
{
    try {
        std::string path;

        if (duk_is_string(ctx, pattern_index))
            path = fs_util::find(base, dukx_get<std::string>(ctx, pattern_index), recursive);
        else {
            // Check if it's a valid RegExp object.
            duk_get_global_string(ctx, "RegExp");
            auto is_regex = duk_instanceof(ctx, pattern_index, -1);
            duk_pop(ctx);

            if (is_regex) {
                duk_get_prop_string(ctx, pattern_index, "source");
                auto pattern = duk_to_string(ctx, -1);
                duk_pop(ctx);

                path = fs_util::find(base, std::regex(pattern), recursive);
            } else
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "pattern must be a string or a regex expression");
        }

        if (path.empty())
            return 0;

        dukx_push(ctx, path);
    } catch (const std::exception& ex) {
        duk_error(ctx, DUK_ERR_ERROR, "%s", ex.what());
    }

    return 1;
}

/*
 * Generic remove function for:
 *
 * - Directory.remove
 * - Directory.prototype.remove
 */
duk_ret_t remove(duk_context* ctx, const std::string& path, bool recursive)
{
    boost::system::error_code ec;

    if (!boost::filesystem::is_directory(path, ec) || ec)
        dukx_throw(ctx, system_error(EINVAL, "not a directory"));

    if (!recursive)
        boost::filesystem::remove(path, ec);
    else
        boost::filesystem::remove_all(path, ec);

    return 0;
}

/*
 * Method: Directory.find(pattern, recursive)
 * --------------------------------------------------------
 *
 * Synonym of Directory.find(path, pattern, recursive) but the path is taken
 * from the directory object.
 *
 * Arguments:
 *   - pattern, the regular expression or file name,
 *   - recursive, set to true to search recursively (default: false).
 * Returns:
 *   The path to the file or undefined if not found.
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_find(duk_context* ctx)
{
    return find(ctx, path(ctx), duk_get_boolean(ctx, 1), 0);
}

/*
 * Method: Directory.remove(recursive)
 * --------------------------------------------------------
 *
 * Synonym of Directory.remove(recursive) but the path is taken from the
 * directory object.
 *
 * Arguments:
 *   - recursive, recursively or not (default: false).
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t method_remove(duk_context* ctx)
{
    return remove(ctx, path(ctx), duk_get_boolean(ctx, 0));
}

const duk_function_list_entry methods[] = {
    { "find",       method_find,    DUK_VARARGS },
    { "remove",     method_remove,  1           },
    { nullptr,      nullptr,        0           }
};

/*
 * Directory "static" functions
 * ------------------------------------------------------------------
 */

/*
 * Function: Irccd.Directory(path) [constructor]
 * --------------------------------------------------------
 *
 * Opens and read the directory at the specified path.
 *
 * Arguments:
 *   - path, the path to the directory,
 * Throws:
 *   - Any exception on error
 */
duk_ret_t constructor(duk_context* ctx)
{
    if (!duk_is_constructor_call(ctx))
        return 0;

    try {
        auto path = duk_require_string(ctx, 0);

        if (!boost::filesystem::is_directory(path))
            dukx_throw(ctx, system_error(EINVAL, "not a directory"));

        duk_push_this(ctx);

        // 'entries' property.
        duk_push_string(ctx, "entries");
        duk_push_array(ctx);

        unsigned i = 0;
        for (const auto& entry : boost::filesystem::directory_iterator(path)) {
            duk_push_object(ctx);
            dukx_push(ctx, entry.path().filename().string());
            duk_put_prop_string(ctx, -2, "name");
            duk_push_int(ctx, entry.status().type());
            duk_put_prop_string(ctx, -2, "type");
            duk_put_prop_index(ctx, -2, i++);
        }

        duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);

        // 'path' property.
        dukx_push(ctx, "path");
        dukx_push(ctx, path);
        duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
    } catch (const std::exception& ex) {
        dukx_throw(ctx, system_error(errno, ex.what()));
    }

    return 0;
}

/*
 * Function: irccd.Directory.find(path, pattern, recursive)
 * --------------------------------------------------------
 *
 * Find an entry by a pattern or a regular expression.
 *
 * Arguments:
 *   - path, the base path,
 *   - pattern, the regular expression or file name,
 *   - recursive, set to true to search recursively (default: false).
 * Returns:
 *   The path to the file or undefined on errors or not found.
 */
duk_ret_t func_find(duk_context* ctx)
{
    return find(ctx, duk_require_string(ctx, 0), duk_get_boolean(ctx, 2), 1);
}

/*
 * Function: irccd.Directory.remove(path, recursive)
 * --------------------------------------------------------
 *
 * Remove the directory optionally recursively.
 *
 * Arguments:
 *   - path, the path to the directory,
 *   - recursive, recursively or not (default: false).
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t func_remove(duk_context *ctx)
{
    return remove(ctx, duk_require_string(ctx, 0), duk_get_boolean(ctx, 1));
}

/*
 * Function: irccd.Directory.mkdir(path, mode = 0700)
 * --------------------------------------------------------
 *
 * Create a directory specified by path. It will create needed subdirectories
 * just like you have invoked mkdir -p.
 *
 * Arguments:
 *   - path, the path to the directory,
 * Throws:
 *   - Any exception on error.
 */
duk_ret_t func_mkdir(duk_context *ctx)
{
    try {
        boost::filesystem::create_directories(duk_require_string(ctx, 0));
    } catch (const std::exception &ex) {
        dukx_throw(ctx, system_error(errno, ex.what()));
    }

    return 0;
}

const duk_function_list_entry functions[] = {
    { "find",           func_find,      DUK_VARARGS },
    { "mkdir",          func_mkdir,     DUK_VARARGS },
    { "remove",         func_remove,    DUK_VARARGS },
    { nullptr,          nullptr,        0           }
};

const duk_number_list_entry constants[] = {
    { "TypeFile",       static_cast<int>(fs::regular_file)    },
    { "TypeDir",        static_cast<int>(fs::directory_file)  },
    { "TypeLink",       static_cast<int>(fs::symlink_file)    },
    { "TypeBlock",      static_cast<int>(fs::block_file)      },
    { "TypeCharacter",  static_cast<int>(fs::character_file)  },
    { "TypeFifo",       static_cast<int>(fs::fifo_file)       },
    { "TypeSocket",     static_cast<int>(fs::socket_file)     },
    { "TypeUnknown",    static_cast<int>(fs::type_unknown)    },
    { nullptr,          0                                           }
};

} // !namespace

std::string directory_jsapi::name() const
{
    return "Irccd.Directory";
}

void directory_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    dukx_stack_assert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_c_function(plugin->context(), constructor, 2);
    duk_put_number_list(plugin->context(), -1, constants);
    duk_put_function_list(plugin->context(), -1, functions);

#if defined(IRCCD_SYSTEM_WINDOWS)
    duk_push_string(plugin->context(), "\\");
#else
    duk_push_string(plugin->context(), "/");
#endif

    duk_put_prop_string(plugin->context(), -2, "separator");

    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, methods);
    duk_put_prop_string(plugin->context(), -2, "prototype");
    duk_put_prop_string(plugin->context(), -2, "Directory");
    duk_pop(plugin->context());
}

} // !irccd

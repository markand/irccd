/*
 * mod-directory.cpp -- Irccd.Directory API
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

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>

#include "duktape.hpp"
#include "fs.hpp"
#include "mod-directory.hpp"
#include "mod-irccd.hpp"
#include "path.hpp"
#include "plugin-js.hpp"
#include "sysconfig.hpp"

namespace irccd {

namespace {

std::string path(duk_context *ctx)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "path");

    if (duk_get_type(ctx, -1) != DUK_TYPE_STRING)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Directory object");

    auto ret = dukx_get_std_string(ctx, -1);

    if (ret.empty())
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "directory object has empty path");

    duk_pop_n(ctx, 2);

    return ret;
}

/*
 * Find an entry recursively (or not) in a directory using a predicate which can
 * be used to test for regular expression, equality.
 *
 * Do not use this function directly, use:
 *
 * - findName
 * - findRegex
 */
template <typename Pred>
std::string findPath(const std::string &base, bool recursive, Pred pred)
{
    /*
     * For performance reason, we first iterate over all entries that are
     * not directories to avoid going deeper recursively if the requested
     * file is in the current directory.
     */
    auto entries = fs::readdir(base);

    for (const auto &entry : entries)
        if (entry.type != fs::Entry::Dir && pred(entry.name))
            return base + entry.name;

    if (!recursive)
        return "";

    for (const auto &entry : entries) {
        if (entry.type == fs::Entry::Dir) {
            std::string next = base + entry.name + fs::separator();
            std::string path = findPath(next, true, pred);

            if (!path.empty())
                return path;
        }
    }

    return "";
}

/*
 * Helper for finding by equality.
 */
std::string findName(std::string base, const std::string &pattern, bool recursive)
{
    return findPath(base, recursive, [&] (const std::string &entryname) -> bool {
        return pattern == entryname;
    });
}

/*
 * Helper for finding by regular expression
 */
std::string findRegex(const std::string &base, std::string pattern, bool recursive)
{
    std::regex regexp(pattern, std::regex::ECMAScript);
    std::smatch smatch;

    return findPath(base, recursive, [&] (const std::string &entryname) -> bool {
        return std::regex_match(entryname, smatch, regexp);
    });
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
duk_ret_t find(duk_context *ctx, std::string base, bool recursive, int patternIndex)
{
    base = path::clean(base);

    try {
        std::string path;

        if (duk_is_string(ctx, patternIndex))
            path = findName(base, duk_get_string(ctx, patternIndex), recursive);
        else {
            // Check if it's a valid RegExp object.
            duk_get_global_string(ctx, "RegExp");
            auto isRegex = duk_instanceof(ctx, patternIndex, -1);
            duk_pop(ctx);

            if (isRegex) {
                duk_get_prop_string(ctx, patternIndex, "source");
                auto pattern = duk_to_string(ctx, -1);
                duk_pop(ctx);

                path = findRegex(base, pattern, recursive);
            } else
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "pattern must be a string or a regex expression");
        }

        if (path.empty())
            return 0;

        dukx_push_std_string(ctx, path);
    } catch (const std::exception &ex) {
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
duk_ret_t remove(duk_context *ctx, const std::string &path, bool recursive)
{
    if (!fs::isDirectory(path))
        dukx_throw(ctx, SystemError(EINVAL, "not a directory"));

    boost::system::error_code ec;

    if (!recursive) {
        boost::filesystem::remove(path, ec);
    } else {
        boost::filesystem::remove_all(path, ec);
    }

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
duk_ret_t methodFind(duk_context *ctx)
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
duk_ret_t methodRemove(duk_context *ctx)
{
    return remove(ctx, path(ctx), duk_get_boolean(ctx, 0));
}

const duk_function_list_entry methods[] = {
    { "find",       methodFind,     DUK_VARARGS },
    { "remove",     methodRemove,   1           },
    { nullptr,      nullptr,        0           }
};

/*
 * Directory "static" functions
 * ------------------------------------------------------------------
 */

/*
 * Function: Irccd.Directory(path, flags) [constructor]
 * --------------------------------------------------------
 *
 * Opens and read the directory at the specified path.
 *
 * Arguments:
 *   - path, the path to the directory,
 *   - flags, the optional flags (default: 0).
 * Throws:
 *   - Any exception on error
 */
duk_ret_t constructor(duk_context *ctx)
{
    if (!duk_is_constructor_call(ctx))
        return 0;

    try {
        std::string path = duk_require_string(ctx, 0);
        std::int8_t flags = duk_get_uint(ctx, 1);

        if (!fs::isDirectory(path))
            dukx_throw(ctx, SystemError(EINVAL, "not a directory"));

        std::vector<fs::Entry> list = fs::readdir(path, flags);

        duk_push_this(ctx);
        duk_push_string(ctx, "count");
        duk_push_int(ctx, list.size());
        duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
        duk_push_string(ctx, "path");
        dukx_push_std_string(ctx, path);
        duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
        duk_push_string(ctx, "entries");
        duk_push_array(ctx);

        for (unsigned i = 0; i < list.size(); ++i) {
            duk_push_object(ctx);
            dukx_push_std_string(ctx, list[i].name);
            duk_put_prop_string(ctx, -2, "name");
            duk_push_int(ctx, list[i].type);
            duk_put_prop_string(ctx, -2, "type");
            duk_put_prop_index(ctx, -2, i);
        }

        duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
    } catch (const std::exception &ex) {
        dukx_throw(ctx, SystemError(errno, ex.what()));
    }

    return 0;
}

/*
 * Function: Irccd.Directory.find(path, pattern, recursive)
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
duk_ret_t funcFind(duk_context *ctx)
{
    return find(ctx, duk_require_string(ctx, 0), duk_get_boolean(ctx, 2), 1);
}

/*
 * Function: Irccd.Directory.remove(path, recursive)
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
duk_ret_t funcRemove(duk_context *ctx)
{
    return remove(ctx, duk_require_string(ctx, 0), duk_get_boolean(ctx, 1));
}

/*
 * Function: Irccd.Directory.mkdir(path, mode = 0700)
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
duk_ret_t funcMkdir(duk_context *ctx)
{
    try {
        boost::filesystem::create_directories(duk_require_string(ctx, 0));
    } catch (const std::exception &ex) {
        dukx_throw(ctx, SystemError(errno, ex.what()));
    }

    return 0;
}

const duk_function_list_entry functions[] = {
    { "find",           funcFind,   DUK_VARARGS },
    { "mkdir",          funcMkdir,  DUK_VARARGS },
    { "remove",         funcRemove, DUK_VARARGS },
    { nullptr,          nullptr,    0           }
};

const duk_number_list_entry constants[] = {
    { "Dot",            static_cast<int>(fs::Dot)               },
    { "DotDot",         static_cast<int>(fs::DotDot)            },
    { "TypeUnknown",    static_cast<int>(fs::Entry::Unknown)    },
    { "TypeDir",        static_cast<int>(fs::Entry::Dir)        },
    { "TypeFile",       static_cast<int>(fs::Entry::File)       },
    { "TypeLink",       static_cast<int>(fs::Entry::Link)       },
    { nullptr,          0                                       }
};

} // !namespace

DirectoryModule::DirectoryModule() noexcept
    : Module("Irccd.Directory")
{
}

void DirectoryModule::load(Irccd &, std::shared_ptr<JsPlugin> plugin)
{
    StackAssert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_c_function(plugin->context(), constructor, 2);
    duk_put_number_list(plugin->context(), -1, constants);
    duk_put_function_list(plugin->context(), -1, functions);
    dukx_push_std_string(plugin->context(), std::string{fs::separator()});
    duk_put_prop_string(plugin->context(), -2, "separator");
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, methods);
    duk_put_prop_string(plugin->context(), -2, "prototype");
    duk_put_prop_string(plugin->context(), -2, "Directory");
    duk_pop(plugin->context());
}

} // !irccd

/*
 * dynlib_plugin.cpp -- native plugin implementation
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

#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/predef/os.h>

#include <irccd/string_util.hpp>

#include "dynlib_plugin.hpp"

#if BOOST_OS_WINDOWS
#   define DYNLIB_EXTENSION ".dll"
#elif BOOST_OS_MACOS
#   define DYNLIB_EXTENSION ".dylib"
#else
#   define DYNLIB_EXTENSION ".so"
#endif

namespace irccd {

namespace {

auto symbol(std::string_view path) -> std::pair<std::string, std::string>
{
    auto id = boost::filesystem::path(std::string(path)).stem().string();

    // Remove forbidden characters.
    id.erase(std::remove_if(id.begin(), id.end(), [] (auto c) {
        return !isalnum(c) && c != '-' && c != '_';
    }), id.end());

    // Transform - to _.
    std::transform(id.begin(), id.end(), id.begin(), [] (auto c) noexcept {
        return c == '-' ? '_' : c;
    });

    return {
        string_util::sprintf("irccd_abi_%s", id),
        string_util::sprintf("irccd_init_%s", id)
    };
}

} // !namespace

dynlib_plugin_loader::dynlib_plugin_loader(std::vector<std::string> directories) noexcept
    : plugin_loader(std::move(directories), { DYNLIB_EXTENSION })
{
}

auto dynlib_plugin_loader::open(std::string_view id, std::string_view path) -> std::shared_ptr<plugin>
{
    const std::string idstr(id);
    const std::string pathstr(path);

    const auto [ abisym, initsym ] = symbol(pathstr);

    using abisym_func_type = version ();
    using initsym_func_type = std::unique_ptr<plugin> ();

    const auto abi = boost::dll::import_alias<abisym_func_type>(pathstr, abisym);
    const auto init = boost::dll::import_alias<initsym_func_type>(pathstr, initsym);

    // The abi version is reset after new major version, check for both.
    const version current;

    if (current.major != abi().major || current.abi != abi().abi)
        throw plugin_error(plugin_error::exec_error, idstr, "incompatible version");

    auto plg = init();

    if (!plg)
        throw plugin_error(plugin_error::exec_error, idstr, "invalid plugin");

    /*
     * We need to keep a reference to `init' variable for the whole plugin
     * lifetime.
     */
    return std::shared_ptr<plugin>(plg.release(), [init] (auto ptr) mutable {
        delete ptr;
    });
}

} // !irccd

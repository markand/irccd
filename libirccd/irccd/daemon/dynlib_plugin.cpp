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

auto symbol(const std::string& path) -> std::pair<std::string, std::string>
{
    auto id = boost::filesystem::path(path).stem().string();

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

auto dynlib_plugin_loader::open(const std::string& id,
                                const std::string& path) -> std::shared_ptr<plugin>
{
    const auto [ abisym, initsym ] = symbol(path);

    using abisym_func_type = unsigned ();
    using initsym_func_type = std::unique_ptr<plugin> ();

    const auto abi = boost::dll::import<abisym_func_type>(path, abisym);
    const auto init = boost::dll::import<initsym_func_type>(path, initsym);

    if (abi() != IRCCD_VERSION_SHLIB)
        throw plugin_error(plugin_error::exec_error, id, "incompatible version");

    auto plg = init();

    if (!plg)
        throw plugin_error(plugin_error::exec_error, id, "invalid plugin");

    /*
     * We need to keep a reference to `init' variable for the whole plugin
     * lifetime.
     */
    return std::shared_ptr<plugin>(plg.release(), [init] (auto ptr) mutable {
        delete ptr;
    });
}

} // !irccd

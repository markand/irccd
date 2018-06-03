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

std::string symbol(const std::string& path) noexcept
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

    return string_util::sprintf("irccd_plugin_%s", id);
}

std::shared_ptr<plugin> wrap(boost::shared_ptr<plugin> ptr) noexcept
{
    return std::shared_ptr<plugin>(ptr.get(), [ptr] (auto) mutable { ptr.reset(); });
}

} // !namespace

dynlib_plugin_loader::dynlib_plugin_loader(std::vector<std::string> directories) noexcept
    : plugin_loader(std::move(directories), { DYNLIB_EXTENSION })
{
}

std::shared_ptr<plugin> dynlib_plugin_loader::open(const std::string&,
                                                   const std::string& path) noexcept
{
    return wrap(boost::dll::import<plugin>(path, symbol(path)));
}

} // !irccd

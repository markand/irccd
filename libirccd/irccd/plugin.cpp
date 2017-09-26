/*
 * plugin.cpp -- irccd JavaScript plugin interface
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

#include <boost/filesystem.hpp>

#include "plugin.hpp"
#include "system.hpp"

namespace fs = boost::filesystem;

namespace irccd {

namespace {

} // !namespace

plugin_loader::plugin_loader(std::vector<std::string> directories,
                             std::vector<std::string> extensions)
    : directories_(std::move(directories))
    , extensions_(std::move(extensions))
{
}

std::shared_ptr<plugin> plugin_loader::find(const std::string& name) noexcept
{
    if (extensions_.empty())
        return nullptr;

    std::vector<std::string> filenames;

    if (directories_.empty())
        filenames = sys::plugin_filenames(name, extensions_);
    else {
        for (const auto& dir : directories_)
            for (const auto& ext : extensions_)
                filenames.push_back(dir + "/" + name + ext);
    }

    std::shared_ptr<plugin> plugin;

    for (const auto& candidate : filenames) {
        boost::system::error_code ec;

        if (!boost::filesystem::exists(candidate, ec) || ec)
            continue;

        plugin = open(name, candidate);

        if (plugin)
            break;
    }

    return plugin;
}

} // !irccd

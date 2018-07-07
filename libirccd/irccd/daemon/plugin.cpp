/*
 * plugin.cpp -- irccd JavaScript plugin interface
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

#include <sstream>

#include <boost/filesystem.hpp>

#include <irccd/system.hpp>

#include "plugin.hpp"

namespace fs = boost::filesystem;

namespace irccd {

std::shared_ptr<plugin> plugin_loader::find(const std::string& name)
{
    std::vector<std::string> filenames;

    if (directories_.empty())
        filenames = sys::plugin_filenames(name, extensions_);
    else {
        for (const auto& dir : directories_)
            for (const auto& ext : extensions_)
                filenames.push_back(dir + "/" + name + ext);
    }

    for (const auto& candidate : filenames) {
        boost::system::error_code ec;

        if (!boost::filesystem::exists(candidate, ec) || ec)
            continue;

        auto plugin = open(name, candidate);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

plugin_error::plugin_error(error errc, std::string name, std::string message) noexcept
    : system_error(make_error_code(errc))
    , name_(std::move(name))
    , message_(std::move(message))
{
    std::ostringstream oss;

    oss << "plugin " << name_ << ": " << code().message();

    std::istringstream iss(message_);
    std::string line;

    while (getline(iss, line))
        oss << "\nplugin " << name_ << ": " << line;

    what_ = oss.str();
}

const std::error_category& plugin_category()
{
    static const class category : public std::error_category {
    public:
        const char* name() const noexcept override
        {
            return "plugin";
        }

        std::string message(int e) const override
        {
            switch (static_cast<plugin_error::error>(e)) {
            case plugin_error::not_found:
                return "plugin not found";
            case plugin_error::invalid_identifier:
                return "invalid identifier";
            case plugin_error::exec_error:
                return "plugin exec error";
            case plugin_error::already_exists:
                return "plugin already exists";
            default:
                return "no error";
            }
        }
    } category;

    return category;
}

std::error_code make_error_code(plugin_error::error e)
{
    return {static_cast<int>(e), plugin_category()};
}

} // !irccd

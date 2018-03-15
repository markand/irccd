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

#include <cctype>
#include <algorithm>

#include <boost/filesystem.hpp>

#include <irccd/string_util.hpp>

#include "dynlib_plugin.hpp"

#if defined(IRCCD_SYSTEM_WINDOWS)
#   define DYNLIB_EXTENSION ".dll"
#elif defined(IRCCD_SYSTEM_MAC)
#   define DYNLIB_EXTENSION ".dylib"
#else
#   define DYNLIB_EXTENSION ".so"
#endif

namespace irccd {

dynlib_plugin::dynlib_plugin(std::string name, std::string path)
    : plugin(name, path)
    , dso_(path)
{
    using load_t = std::unique_ptr<plugin>(std::string, std::string);

    /*
     * Function name is determined from the plugin filename where all non
     * alphabetic characters are removed.
     *
     * Example: foo_bar-baz___.so becomes irccd_foobarbaz_load.
     */
    auto base = boost::filesystem::path(path).stem().string();
    auto need_remove = [] (auto c) {
        return !std::isalnum(c);
    };

    base.erase(std::remove_if(base.begin(), base.end(), need_remove), base.end());

    auto fname = string_util::sprintf("irccd_%s_load", base);
    auto load = dso_.get<load_t>(fname);

    if (!load)
        throw std::runtime_error(string_util::sprintf("missing plugin entry function '%s'", fname));

    plugin_ = load(name, path);

    if (!plugin_)
        throw std::runtime_error("plugin returned null");
}

void dynlib_plugin::handle_command(irccd& irccd, const message_event& ev)
{
    plugin_->handle_command(irccd, ev);
}

void dynlib_plugin::handle_connect(irccd& irccd, const connect_event& ev)
{
    plugin_->handle_connect(irccd, ev);
}

void dynlib_plugin::handle_disconnect(irccd& irccd, const disconnect_event& ev)
{
    plugin_->handle_disconnect(irccd, ev);
}

void dynlib_plugin::handle_invite(irccd& irccd, const invite_event& ev)
{
    plugin_->handle_invite(irccd, ev);
}

void dynlib_plugin::handle_join(irccd& irccd, const join_event& ev)
{
    plugin_->handle_join(irccd, ev);
}

void dynlib_plugin::handle_kick(irccd& irccd, const kick_event& ev)
{
    plugin_->handle_kick(irccd, ev);
}

void dynlib_plugin::handle_load(irccd& irccd)
{
    plugin_->handle_load(irccd);
}

void dynlib_plugin::handle_message(irccd& irccd, const message_event& ev)
{
    plugin_->handle_message(irccd, ev);
}

void dynlib_plugin::handle_me(irccd& irccd, const me_event& ev)
{
    plugin_->handle_me(irccd, ev);
}

void dynlib_plugin::handle_mode(irccd& irccd, const mode_event& ev)
{
    plugin_->handle_mode(irccd, ev);
}

void dynlib_plugin::handle_names(irccd& irccd, const names_event& ev)
{
    plugin_->handle_names(irccd, ev);
}

void dynlib_plugin::handle_nick(irccd& irccd, const nick_event& ev)
{
    plugin_->handle_nick(irccd, ev);
}

void dynlib_plugin::handle_notice(irccd& irccd, const notice_event& ev)
{
    plugin_->handle_notice(irccd, ev);
}

void dynlib_plugin::handle_part(irccd& irccd, const part_event& ev)
{
    plugin_->handle_part(irccd, ev);
}

void dynlib_plugin::handle_reload(irccd& irccd)
{
    plugin_->handle_reload(irccd);
}

void dynlib_plugin::handle_topic(irccd& irccd, const topic_event& ev)
{
    plugin_->handle_topic(irccd, ev);
}

void dynlib_plugin::handle_unload(irccd& irccd)
{
    plugin_->handle_unload(irccd);
}

void dynlib_plugin::handle_whois(irccd& irccd, const whois_event& ev)
{
    plugin_->handle_whois(irccd, ev);
}

dynlib_plugin_loader::dynlib_plugin_loader(std::vector<std::string> directories) noexcept
    : plugin_loader(std::move(directories), { DYNLIB_EXTENSION })
{
}

std::shared_ptr<plugin> dynlib_plugin_loader::open(const std::string& id,
                                                   const std::string& path) noexcept
{
    return std::make_unique<dynlib_plugin>(id, path);
}

} // !irccd

/*
 * links.cpp -- links plugin
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

#include <boost/dll.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server.hpp>

#include <irccd/string_util.hpp>

#include "requester.hpp"
#include "links.hpp"

using std::make_unique;
using std::move;
using std::string;
using std::string_view;
using std::unique_ptr;

namespace irccd {

using string_util::to_uint;

auto links_plugin::get_name() const noexcept -> string_view
{
    return "links";
}

auto links_plugin::get_author() const noexcept -> string_view
{
    return "David Demelier <markand@malikania.fr>";
}

auto links_plugin::get_license() const noexcept -> string_view
{
    return "ISC";
}

auto links_plugin::get_summary() const noexcept -> string_view
{
    return "show webpage title";
}

auto links_plugin::get_version() const noexcept -> string_view
{
    return IRCCD_VERSION;
}

void links_plugin::set_options(const map& conf)
{
    if (const auto it = conf.find("timeout"); it != conf.end())
        if (const auto v = to_uint(it->second); v)
            conf_timeout = *v;
}

void links_plugin::set_formats(const map& formats)
{
    if (const auto it = formats.find("info"); it != formats.end())
        format_info = it->second;
}

void links_plugin::handle_message(irccd& irccd, const message_event& ev)
{
    requester::run(irccd.get_service(), ev.server, ev.origin, ev.channel, ev.message);
}

auto links_plugin::abi() -> version
{
    return version();
}

auto links_plugin::init(string id) -> unique_ptr<plugin>
{
    return make_unique<links_plugin>(move(id));
}

BOOST_DLL_ALIAS(links_plugin::abi, irccd_abi_links)
BOOST_DLL_ALIAS(links_plugin::init, irccd_init_links)

// }}}

} // !irccd

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

#include <cassert>
#include <sstream>

#include <boost/filesystem.hpp>

#include <irccd/system.hpp>
#include <irccd/string_util.hpp>

#include "plugin.hpp"

namespace irccd {

plugin::plugin(std::string id) noexcept
    : id_(std::move(id))
{
    assert(string_util::is_identifier(id_));
}

auto plugin::get_id() const noexcept -> const std::string&
{
    return id_;
}

auto plugin::get_author() const noexcept -> std::string_view
{
    return "unknown";
}

auto plugin::get_license() const noexcept -> std::string_view
{
    return "unknown";
}

auto plugin::get_summary() const noexcept -> std::string_view
{
    return "unknown";
}

auto plugin::get_version() const noexcept -> std::string_view
{
    return "unknown";
}

auto plugin::get_options() const -> map
{
    return {};
}

void plugin::set_options(const map&)
{
}

auto plugin::get_formats() const -> map
{
    return {};
}

void plugin::set_formats(const map&)
{
}

auto plugin::get_paths() const -> map
{
    return {};
}

void plugin::set_paths(const map&)
{
}

void plugin::handle_command(irccd&, const message_event&)
{
}

void plugin::handle_connect(irccd&, const connect_event&)
{
}

void plugin::handle_disconnect(irccd&, const disconnect_event&)
{
}

void plugin::handle_invite(irccd&, const invite_event&)
{
}

void plugin::handle_join(irccd&, const join_event&)
{
}

void plugin::handle_kick(irccd&, const kick_event&)
{
}

void plugin::handle_load(irccd&)
{
}

void plugin::handle_message(irccd&, const message_event&)
{
}

void plugin::handle_me(irccd&, const me_event&)
{
}

void plugin::handle_mode(irccd&, const mode_event&)
{
}

void plugin::handle_names(irccd&, const names_event&)
{
}

void plugin::handle_nick(irccd&, const nick_event&)
{
}

void plugin::handle_notice(irccd&, const notice_event&)
{
}

void plugin::handle_part(irccd&, const part_event&)
{
}

void plugin::handle_reload(irccd&)
{
}

void plugin::handle_topic(irccd&, const topic_event&)
{
}

void plugin::handle_unload(irccd&)
{
}

void plugin::handle_whois(irccd&, const whois_event&)
{
}

plugin_loader::plugin_loader(std::vector<std::string> directories,
              std::vector<std::string> extensions) noexcept
    : directories_(std::move(directories))
    , extensions_(std::move(extensions))
{
    assert(!extensions_.empty());
}

auto plugin_loader::find(std::string_view name) -> std::shared_ptr<plugin>
{
    std::vector<std::string> filenames;

    if (directories_.empty())
        filenames = sys::plugin_filenames(std::string(name), extensions_);
    else {
        for (const auto& dir : directories_)
            for (const auto& ext : extensions_)
                filenames.push_back(dir + std::string("/") + std::string(name) + ext);
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

plugin_error::plugin_error(error errc, std::string_view name, std::string_view message)
    : system_error(make_error_code(errc))
    , name_(std::move(name))
    , message_(std::move(message))
{
    std::ostringstream oss;

    oss << code().message();

    std::istringstream iss(message_);
    std::string line;

    while (getline(iss, line))
        oss << "\n" << line;

    what_ = oss.str();
}

auto plugin_error::get_name() const noexcept -> const std::string&
{
    return name_;
}

auto plugin_error::get_message() const noexcept -> const std::string&
{
    return message_;
}

auto plugin_error::what() const noexcept -> const char*
{
    return what_.c_str();
}

auto plugin_category() -> const std::error_category&
{
    static const class category : public std::error_category {
    public:
        auto name() const noexcept -> const char* override
        {
            return "plugin";
        }

        auto message(int e) const -> std::string override
        {
            switch (static_cast<plugin_error::error>(e)) {
            case plugin_error::not_found:
                return "plugin not found";
            case plugin_error::invalid_identifier:
                return "invalid plugin identifier";
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

auto make_error_code(plugin_error::error e) -> std::error_code
{
    return {static_cast<int>(e), plugin_category()};
}

} // !irccd

/*
 * server_util.cpp -- server utilities
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

#include <irccd/config.hpp>
#include <irccd/ini.hpp>
#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>

#include "server_util.hpp"

namespace irccd {

namespace server_util {

namespace {

template <typename T>
T to_int(const std::string& value, const std::string& name, server_error::error errc)
{
    try {
        return string_util::to_int<T>(value);
    } catch (...) {
        throw server_error(errc, name);
    }
}

template <typename T>
T to_uint(const std::string& value, const std::string& name, server_error::error errc)
{
    try {
        return string_util::to_uint<T>(value);
    } catch (...) {
        throw server_error(errc, name);
    }
}

template <typename T>
T to_uint(const nlohmann::json& value, const std::string& name, server_error::error errc)
{
    if (!value.is_number())
        throw server_error(errc, name);

    auto n = value.get<unsigned>();

    if (n > std::numeric_limits<T>::max())
        throw server_error(errc, name);

    return static_cast<T>(n);
}

std::string to_id(const ini::section& sc)
{
    auto id = sc.get("name");

    if (!string_util::is_identifier(id.value()))
        throw server_error(server_error::invalid_identifier, "");

    return id.value();
}

std::string to_id(const nlohmann::json& object)
{
    auto id = json_util::get_string(object, "name");

    if (!string_util::is_identifier(id))
        throw server_error(server_error::invalid_identifier, "");

    return id;
}

std::string to_host(const ini::section& sc, const std::string& name)
{
    auto value = sc.get("host");

    if (value.empty())
        throw server_error(server_error::invalid_hostname, name);

    return value.value();
}

std::string to_host(const nlohmann::json& object, const std::string& name)
{
    auto value = json_util::get_string(object, "host");

    if (value.empty())
        throw server_error(server_error::invalid_hostname, name);

    return value;
}

void load_identity(server& server, const config& cfg, const std::string& identity)
{
    auto sc = std::find_if(cfg.doc().begin(), cfg.doc().end(), [&] (const auto& sc) {
        if (sc.key() != "identity")
            return false;

        auto name = sc.find("name");

        return name != sc.end() && name->value() == identity;
    });

    if (sc == cfg.doc().end())
        return;

    ini::section::const_iterator it;

    if ((it = sc->find("username")) != sc->end())
        server.set_username(it->value());
    if ((it = sc->find("realname")) != sc->end())
        server.set_realname(it->value());
    if ((it = sc->find("nickname")) != sc->end())
        server.set_nickname(it->value());
    if ((it = sc->find("ctcp-version")) != sc->end())
        server.set_ctcp_version(it->value());
}

} // !namespace

std::shared_ptr<server> from_json(boost::asio::io_service& service, const nlohmann::json& object)
{
    // TODO: move this function in server_service.
    auto sv = std::make_shared<server>(service, to_id(object));

    // Mandatory fields.
    sv->set_host(to_host(object, sv->name()));

    // Optional fields.
    if (object.count("port"))
        sv->set_port(to_uint<std::uint16_t>(object["port"], sv->name(), server_error::invalid_port));
    sv->set_password(json_util::get_string(object, "password"));
    sv->set_nickname(json_util::get_string(object, "nickname", sv->nickname()));
    sv->set_realname(json_util::get_string(object, "realname", sv->realname()));
    sv->set_username(json_util::get_string(object, "username", sv->username()));
    sv->set_ctcp_version(json_util::get_string(object, "ctcpVersion", sv->ctcp_version()));
    sv->set_command_char(json_util::get_string(object, "commandChar", sv->command_char()));

    if (json_util::get_bool(object, "ipv6"))
        sv->set_flags(sv->flags() | server::ipv6);
    if (json_util::get_bool(object, "sslVerify"))
        sv->set_flags(sv->flags() | server::ssl_verify);
    if (json_util::get_bool(object, "autoRejoin"))
        sv->set_flags(sv->flags() | server::auto_rejoin);
    if (json_util::get_bool(object, "joinInvite"))
        sv->set_flags(sv->flags() | server::join_invite);

    if (json_util::get_bool(object, "ssl"))
#if defined(HAVE_SSL)
        sv->set_flags(sv->flags() | server::ssl);
#else
        throw server_error(server_error::ssl_disabled, sv->name());
#endif

    return sv;
}

std::shared_ptr<server> from_config(boost::asio::io_service& service,
                                    const config& cfg,
                                    const ini::section& sc)
{
    assert(sc.key() == "server");

    auto sv = std::make_shared<server>(service, to_id(sc));

    // Mandatory fields.
    sv->set_host(to_host(sc, sv->name()));

    // Optional fields.
    ini::section::const_iterator it;

    if ((it = sc.find("password")) != sc.end())
        sv->set_password(it->value());

    // Optional flags
    if ((it = sc.find("ipv6")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::ipv6);

    if ((it = sc.find("ssl")) != sc.end() && string_util::is_boolean(it->value())) {
#if defined(HAVE_SSL)
        sv->set_flags(sv->flags() | server::ssl);
#else
        throw server_error(server_error::ssl_disabled, sv->name());
#endif
    }

    if ((it = sc.find("ssl-verify")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::ssl_verify);

    // Optional identity
    if ((it = sc.find("identity")) != sc.end())
        load_identity(*sv, cfg, it->value());

    // Options
    if ((it = sc.find("auto-rejoin")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::auto_rejoin);
    if ((it = sc.find("join-invite")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::join_invite);

    // Channels
    if ((it = sc.find("channels")) != sc.end()) {
        for (const auto& s : *it) {
            channel channel;

            if (auto pos = s.find(":") != std::string::npos) {
                channel.name = s.substr(0, pos);
                channel.password = s.substr(pos + 1);
            } else
                channel.name = s;

            sv->join(channel.name, channel.password);
        }
    }
    if ((it = sc.find("command-char")) != sc.end())
        sv->set_command_char(it->value());

    // Reconnect and ping timeout
    if ((it = sc.find("port")) != sc.end())
        sv->set_port(to_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_port));

    if ((it = sc.find("reconnect-tries")) != sc.end())
        sv->set_reconnect_tries(to_int<std::int8_t>(it->value(),
            sv->name(), server_error::invalid_reconnect_tries));

    if ((it = sc.find("reconnect-timeout")) != sc.end())
        sv->set_reconnect_delay(to_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_reconnect_timeout));

    if ((it = sc.find("ping-timeout")) != sc.end())
        sv->set_ping_timeout(to_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_ping_timeout));

    return sv;
}

} // !server_util

} // !irccd

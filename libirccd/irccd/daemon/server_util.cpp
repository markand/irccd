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

using nlohmann::json;

using std::uint16_t;
using std::string;
using std::forward;

namespace irccd {

namespace server_util {

namespace {

template <typename... Args>
std::string require_conf_host(const ini::section& sc, Args&&... args)
{
    auto value = sc.get("host");

    if (value.empty())
        throw server_error(forward<Args>(args)...);

    return value.value();
}

template <typename... Args>
std::string require_conf_id(const ini::section& sc, Args&&... args)
{
    auto id = sc.get("name");

    if (!string_util::is_identifier(id.value()))
        throw server_error(forward<Args>(args)...);

    return id.value();
}

template <typename Int, typename... Args>
Int optional_conf_int(const string& value, Args&&... args)
{
    try {
        return string_util::to_int<Int>(value);
    } catch (...) {
        throw server_error(std::forward<Args>(args)...);
    }
}

template <typename Int, typename... Args>
Int optional_conf_uint(const string& value, Args&&... args)
{
    try {
        return string_util::to_uint<Int>(value);
    } catch (...) {
        throw server_error(std::forward<Args>(args)...);
    }
}

template <typename Int, typename... Args>
Int optional_json_uint(const json& json, const json::json_pointer& key, Int def, Args&&... args)
{
    const auto v = json_util::optional_uint(json, key, def);

    if (!v || *v > std::numeric_limits<Int>::max())
        throw server_error(forward<Args>(args)...);

    return *v;
}

bool optional_json_bool(const json& json, const json::json_pointer& key, bool def = false)
{
    const auto v = json_util::optional_bool(json, key, def);

    if (!v)
        return def;

    return *v;
}

template <typename... Args>
std::string optional_json_string(const json& json,
                                 const json::json_pointer& key,
                                 const string& def,
                                 Args&&... args)
{
    const auto v = json_util::optional_string(json, key, def);

    if (!v)
        throw server_error(forward<Args>(args)...);

    return *v;
}

template <typename... Args>
std::string require_json_id(const nlohmann::json& json, Args&&... args)
{
    const auto id = json_util::get_string(json, "/name"_json_pointer);

    if (!id || !string_util::is_identifier(*id))
        throw server_error(forward<Args>(args)...);

    return *id;
}

template <typename... Args>
std::string require_json_host(const nlohmann::json& object, Args&&... args)
{
    const auto value = json_util::get_string(object, "/host"_json_pointer);

    if (!value || value->empty())
        throw server_error(forward<Args>(args)...);

    return *value;
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
    const auto id = require_json_id(object, "", server_error::invalid_identifier);
    const auto sv = std::make_shared<server>(service, id);

    // Mandatory fields.
    sv->set_host(require_json_host(object, sv->name(), server_error::invalid_hostname));

    // Optional fields.
    sv->set_port(optional_json_uint<uint16_t>(object, "/port"_json_pointer, sv->port(),
        sv->name(), server_error::invalid_port));
    sv->set_password(optional_json_string(object, "/password"_json_pointer, sv->password(),
        sv->name(), server_error::invalid_password));
    sv->set_nickname(optional_json_string(object, "/nickname"_json_pointer, sv->nickname(),
        sv->name(), server_error::invalid_nickname));
    sv->set_realname(optional_json_string(object, "/realname"_json_pointer, sv->realname(),
        sv->name(), server_error::invalid_realname));
    sv->set_username(optional_json_string(object, "/username"_json_pointer, sv->username(),
        sv->name(), server_error::invalid_username));
    sv->set_ctcp_version(optional_json_string(object, "/ctcpVersion"_json_pointer, sv->ctcp_version(),
        sv->name(), server_error::invalid_ctcp_version));
    sv->set_command_char(optional_json_string(object, "/commandChar"_json_pointer, sv->command_char(),
        sv->name(), server_error::invalid_command_char));

    // Boolean does not throw options though.
    if (optional_json_bool(object, "/ipv6"_json_pointer))
        sv->set_flags(sv->flags() | server::ipv6);
    if (optional_json_bool(object, "/sslVerify"_json_pointer))
        sv->set_flags(sv->flags() | server::ssl_verify);
    if (optional_json_bool(object, "/autoRejoin"_json_pointer))
        sv->set_flags(sv->flags() | server::auto_rejoin);
    if (optional_json_bool(object, "/joinInvite"_json_pointer))
        sv->set_flags(sv->flags() | server::join_invite);

    if (optional_json_bool(object, "/ssl"_json_pointer))
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

    const auto id = require_conf_id(sc, "", server_error::invalid_identifier);
    const auto sv = std::make_shared<server>(service, id);

    // Mandatory fields.
    sv->set_host(require_conf_host(sc, sv->name(), server_error::invalid_hostname));

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
        sv->set_port(optional_conf_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_port));

    if ((it = sc.find("reconnect-tries")) != sc.end())
        sv->set_reconnect_tries(optional_conf_int<std::int8_t>(it->value(),
            sv->name(), server_error::invalid_reconnect_tries));

    if ((it = sc.find("reconnect-timeout")) != sc.end())
        sv->set_reconnect_delay(optional_conf_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_reconnect_timeout));

    if ((it = sc.find("ping-timeout")) != sc.end())
        sv->set_ping_timeout(optional_conf_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_ping_timeout));

    return sv;
}

std::string get_identifier(const nlohmann::json& json)
{
    const auto v = json_util::get_string(json, "/server"_json_pointer);

    if (!v)
        throw server_error("", server_error::invalid_identifier);
    if (!string_util::is_identifier(*v))
        throw server_error(*v, server_error::invalid_identifier);

    return *v;
}

} // !server_util

} // !irccd

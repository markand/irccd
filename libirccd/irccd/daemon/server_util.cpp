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
#include <irccd/ini_util.hpp>
#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>

#include "server_util.hpp"

namespace irccd {

namespace server_util {

namespace {

void from_config_load_identity(server& sv, const ini::section& sc)
{
    const auto username = ini_util::optional_string(sc, "username", sv.get_username());
    const auto realname = ini_util::optional_string(sc, "realname", sv.get_realname());
    const auto nickname = ini_util::optional_string(sc, "nickname", sv.get_nickname());
    const auto ctcp_version = ini_util::optional_string(sc, "ctcp-version", sv.get_ctcp_version());

    if (username.empty())
        throw server_error(server_error::invalid_username);
    if (realname.empty())
        throw server_error(server_error::invalid_realname);
    if (nickname.empty())
        throw server_error(server_error::invalid_nickname);
    if (ctcp_version.empty())
        throw server_error(server_error::invalid_ctcp_version);

    sv.set_username(username);
    sv.set_realname(realname);
    sv.set_nickname(nickname);
    sv.set_ctcp_version(ctcp_version);
}

void from_config_load_channels(server& sv, const ini::section& sc)
{
    for (const auto& s : sc.get("channels")) {
        channel channel;

        if (auto pos = s.find(":") != std::string::npos) {
            channel.name = s.substr(0, pos);
            channel.password = s.substr(pos + 1);
        } else
            channel.name = s;

        sv.join(channel.name, channel.password);
    }
}

void from_config_load_flags(server& sv, const ini::section& sc)
{
    const auto ipv6 = sc.get("ipv6");
    const auto ssl = sc.get("ssl");
    const auto ssl_verify = sc.get("ssl-verify");
    const auto auto_rejoin = sc.get("auto-rejoin");
    const auto join_invite = sc.get("join-invite");

    if (string_util::is_boolean(ipv6.value()))
        sv.set_flags(sv.get_flags() | server::ipv6);
    if (string_util::is_boolean(ssl.value()))
        sv.set_flags(sv.get_flags() | server::ssl);
    if (string_util::is_boolean(ssl_verify.value()))
        sv.set_flags(sv.get_flags() | server::ssl_verify);
    if (string_util::is_boolean(auto_rejoin.value()))
        sv.set_flags(sv.get_flags() | server::auto_rejoin);
    if (string_util::is_boolean(join_invite.value()))
        sv.set_flags(sv.get_flags() | server::join_invite);
}

void from_config_load_numeric_parameters(server& sv, const ini::section& sc)
{
    const auto port = ini_util::optional_uint<std::uint16_t>(sc, "port", sv.get_port());
    const auto ping_timeout = ini_util::optional_uint<uint16_t>(sc, "ping-timeout", sv.get_ping_timeout());
    const auto reco_tries = ini_util::optional_uint<uint8_t>(sc, "reconnect-tries", sv.get_reconnect_tries());
    const auto reco_timeout = ini_util::optional_uint<uint16_t>(sc, "reconnect-delay", sv.get_reconnect_delay());

    if (!port)
        throw server_error(server_error::invalid_port);
    if (!ping_timeout)
        throw server_error(server_error::invalid_ping_timeout);
    if (!reco_tries)
        throw server_error(server_error::invalid_reconnect_tries);
    if (!reco_timeout)
        throw server_error(server_error::invalid_reconnect_timeout);

    sv.set_port(*port);
    sv.set_ping_timeout(*ping_timeout);
    sv.set_reconnect_tries(*reco_tries);
    sv.set_reconnect_delay(*reco_timeout);
}

void from_config_load_options(server& sv, const ini::section& sc)
{
    const auto password = ini_util::optional_string(sc, "password", "");
    const auto command_char = ini_util::optional_string(sc, "command-char", sv.get_command_char());

    sv.set_password(password);
    sv.set_command_char(command_char);
}

void from_json_load_options(server& sv, const nlohmann::json& object)
{
    const auto port = json_util::optional_uint(object, "port", sv.get_port());
    const auto nickname = json_util::optional_string(object, "nickname", sv.get_nickname());
    const auto realname = json_util::optional_string(object, "realname", sv.get_realname());
    const auto username = json_util::optional_string(object, "username", sv.get_username());
    const auto ctcp_version = json_util::optional_string(object, "ctcpVersion", sv.get_ctcp_version());
    const auto command = json_util::optional_string(object, "commandChar", sv.get_command_char());
    const auto password = json_util::optional_string(object, "password", sv.get_password());

    if (!port || *port > std::numeric_limits<std::uint16_t>::max())
        throw server_error(server_error::invalid_port);
    if (!nickname)
        throw server_error(server_error::invalid_nickname);
    if (!realname)
        throw server_error(server_error::invalid_realname);
    if (!username)
        throw server_error(server_error::invalid_username);
    if (!ctcp_version)
        throw server_error(server_error::invalid_ctcp_version);
    if (!command)
        throw server_error(server_error::invalid_command_char);
    if (!password)
        throw server_error(server_error::invalid_password);

    sv.set_port(*port);
    sv.set_nickname(*nickname);
    sv.set_realname(*realname);
    sv.set_username(*username);
    sv.set_ctcp_version(*ctcp_version);
    sv.set_command_char(*command);
    sv.set_password(*password);
}

void from_json_load_flags(server& sv, nlohmann::json object)
{
    const auto ipv6 = object["ipv6"];
    const auto ssl = object["ssl"];
    const auto ssl_verify = object["sslVerify"];
    const auto auto_rejoin = object["autoRejoin"];
    const auto join_invite = object["joinInvite"];

    if (ipv6.is_boolean() && ipv6.get<bool>())
        sv.set_flags(sv.get_flags() | server::ipv6);
    if (ssl.is_boolean() && ssl.get<bool>())
        sv.set_flags(sv.get_flags() | server::ssl);
    if (ssl_verify.is_boolean() && ssl_verify.get<bool>())
        sv.set_flags(sv.get_flags() | server::ssl_verify);
    if (auto_rejoin.is_boolean() && auto_rejoin.get<bool>())
        sv.set_flags(sv.get_flags() | server::auto_rejoin);
    if (join_invite.is_boolean() && join_invite.get<bool>())
        sv.set_flags(sv.get_flags() | server::join_invite);

#if !defined(HAVE_SSL)
    if (sv.get_flags() & server::ssl)
        throw server_error(server_error::ssl_disabled);
#endif
}

} // !namespace

std::shared_ptr<server> from_json(boost::asio::io_service& service, const nlohmann::json& object)
{
    // Mandatory parameters.
    const auto id = json_util::get_string(object, "name");
    const auto host = json_util::get_string(object, "host");

    if (!id || !string_util::is_identifier(*id))
        throw server_error(server_error::invalid_identifier);
    if (!host || host->empty())
        throw server_error(server_error::invalid_hostname);

    const auto sv = std::make_shared<server>(service, *id, *host);

    from_json_load_options(*sv, object);
    from_json_load_flags(*sv, object);

    return sv;
}

std::shared_ptr<server> from_config(boost::asio::io_service& service,
                                    const config& cfg,
                                    const ini::section& sc)
{
    // Mandatory parameters.
    const auto id = sc.get("name");
    const auto host = sc.get("hostname");

    if (!string_util::is_identifier(id.value()))
        throw server_error(server_error::invalid_identifier);
    if (host.value().empty())
        throw server_error(server_error::invalid_hostname);

    const auto sv = std::make_shared<server>(service, id.value(), host.value());

    from_config_load_channels(*sv, sc);
    from_config_load_flags(*sv, sc);
    from_config_load_numeric_parameters(*sv, sc);
    from_config_load_options(*sv, sc);

    // Identity is in a separate section
    const auto identity = sc.get("identity");

    if (identity.value().size() > 0) {
        const auto it = std::find_if(cfg.doc().begin(), cfg.doc().end(), [&] (const auto& i) {
            return i.get("name").value() == identity.value();
        });

        if (it != cfg.doc().end())
            from_config_load_identity(*sv, sc);
    }

    return sv;
}

} // !server_util

} // !irccd

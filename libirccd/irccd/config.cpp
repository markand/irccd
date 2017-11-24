/*
 * config.cpp -- irccd configuration loader
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

#include <cassert>

#include <boost/filesystem.hpp>

#include "config.hpp"
#include "irccd.hpp"
#include "logger.hpp"
#include "plugin_service.hpp"
#include "rule.hpp"
#include "server.hpp"
#include "string_util.hpp"
#include "sysconfig.hpp"
#include "system.hpp"
#include "transport_server.hpp"
#include "transport_service.hpp"

namespace irccd {

namespace {

class irccd_log_filter : public log::filter {
private:
    std::string convert(const std::string& tmpl, std::string input) const
    {
        if (tmpl.empty())
            return input;

        string_util::subst params;

        params.flags &= ~(string_util::subst_flags::irc_attrs);
        params.keywords.emplace("message", std::move(input));

        return string_util::format(tmpl, params);
    }

public:
    std::string debug_;
    std::string info_;
    std::string warning_;

    std::string pre_debug(std::string input) const override
    {
        return convert(debug_, std::move(input));
    }

    std::string pre_info(std::string input) const override
    {
        return convert(info_, std::move(input));
    }

    std::string pre_warning(std::string input) const override
    {
        return convert(warning_, std::move(input));
    }
};

std::string get(const ini::document& doc, const std::string& section, const std::string& key)
{
    auto its = doc.find(section);

    if (its == doc.end())
        return "";

    auto ito = its->find(key);

    if (ito == its->end())
        return "";

    return ito->value();
}

std::unique_ptr<log::logger> load_log_file(const ini::section& sc)
{
    /*
     * TODO: improve that with CMake options.
     */
#if defined(IRCCD_SYSTEM_WINDOWS)
    std::string normal = "log.txt";
    std::string errors = "errors.txt";
#else
    std::string normal = "/var/log/irccd/log.txt";
    std::string errors = "/var/log/irccd/errors.txt";
#endif

    ini::section::const_iterator it;

    if ((it = sc.find("path-logs")) != sc.end())
        normal = it->value();
    if ((it = sc.find("path-errors")) != sc.end())
        errors = it->value();

    return std::make_unique<log::file_logger>(std::move(normal), std::move(errors));
}

std::unique_ptr<log::logger> load_log_syslog()
{
#if defined(HAVE_SYSLOG)
    return std::make_unique<log::syslog_logger>();
#else
    throw std::runtime_error("logs: syslog is not available on this platform");
#endif // !HAVE_SYSLOG
}

std::unique_ptr<transport_server> load_transport_ip(boost::asio::io_service& service, const ini::section& sc)
{
    assert(sc.key() == "transport");

    std::unique_ptr<transport_server> transport;
    ini::section::const_iterator it;

    // Port.
    int port;

    if ((it = sc.find("port")) == sc.cend())
        throw std::invalid_argument("transport: missing 'port' parameter");

    try {
        port = string_util::to_uint<std::uint16_t>(it->value());
    } catch (const std::exception&) {
        throw std::invalid_argument(string_util::sprintf("transport: invalid port number: %s", it->value()));
    }

    // Address.
    std::string address = "*";

    if ((it = sc.find("address")) != sc.end())
        address = it->value();

    // 0011
    //    ^ define IPv4
    //   ^  define IPv6
    auto mode = 1U;

    /*
     * Documentation stated family but code checked for 'domain' option.
     *
     * As irccdctl uses domain, accept both and unify the option name to 'family'.
     *
     * See #637
     */
    if ((it = sc.find("domain")) != sc.end() || (it = sc.find("family")) != sc.end()) {
        for (const auto& v : *it) {
            if (v == "ipv4")
                mode |= (1U << 0);
            if (v == "ipv6")
                mode |= (1U << 1);
        }
    }

    if (mode == 0U)
        throw std::invalid_argument("transport: family must at least have ipv4 or ipv6");

    auto protocol = (mode & 0x2U)
        ? boost::asio::ip::tcp::v4()
        : boost::asio::ip::tcp::v6();

    // Optional SSL.
    std::string pkey;
    std::string cert;

    if ((it = sc.find("ssl")) != sc.end() && string_util::is_boolean(it->value())) {
        if ((it = sc.find("certificate")) == sc.end())
            throw std::invalid_argument("transport: missing 'certificate' parameter");

        cert = it->value();

        if ((it = sc.find("key")) == sc.end())
            throw std::invalid_argument("transport: missing 'key' parameter");

        pkey = it->value();
    }

    auto endpoint = (address == "*")
        ? boost::asio::ip::tcp::endpoint(protocol, port)
        : boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port);

    boost::asio::ip::tcp::acceptor acceptor(service, endpoint, true);

    if (pkey.empty())
        return std::make_unique<ip_transport_server>(std::move(acceptor));

#if defined(HAVE_SSL)
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

    ctx.use_private_key_file(pkey, boost::asio::ssl::context::pem);
    ctx.use_certificate_file(cert, boost::asio::ssl::context::pem);

    return std::make_unique<tls_transport_server>(std::move(acceptor), std::move(ctx));
#else
    throw std::invalid_argument("transport: SSL disabled");
#endif
}

std::unique_ptr<transport_server> load_transport_unix(boost::asio::io_service& service, const ini::section& sc)
{
    using boost::asio::local::stream_protocol;

    assert(sc.key() == "transport");

#if !defined(IRCCD_SYSTEM_WINDOWS)
    ini::section::const_iterator it = sc.find("path");

    if (it == sc.end())
        throw std::invalid_argument("transport: missing 'path' parameter");

    // Remove the file first.
    std::remove(it->value().c_str());

    stream_protocol::endpoint endpoint(it->value());
    stream_protocol::acceptor acceptor(service, std::move(endpoint));

    return std::make_unique<local_transport_server>(std::move(acceptor));
#else
    (void)sc;

    throw std::invalid_argument("transport: unix transport not supported on on this platform");
#endif
}

std::unique_ptr<transport_server> load_transport(boost::asio::io_service& service, const ini::section& sc)
{
    assert(sc.key() == "transport");

    std::unique_ptr<transport_server> transport;
    ini::section::const_iterator it = sc.find("type");

    if (it == sc.end())
        throw std::invalid_argument("transport: missing 'type' parameter");

    if (it->value() == "ip")
        transport = load_transport_ip(service, sc);
    else if (it->value() == "unix")
        transport = load_transport_unix(service, sc);
    else
        throw std::invalid_argument(string_util::sprintf("transport: invalid type given: %s", it->value()));

    if ((it = sc.find("password")) != sc.end())
        transport->set_password(it->value());

    return transport;
}

rule load_rule(const ini::section& sc)
{
    assert(sc.key() == "rule");

    // Simple converter from std::vector to std::unordered_set.
    auto toset = [] (const auto& v) {
        return std::unordered_set<std::string>(v.begin(), v.end());
    };

    rule::set servers, channels, origins, plugins, events;
    rule::action_type action = rule::action_type::accept;

    // Get the sets.
    ini::section::const_iterator it;

    if ((it = sc.find("servers")) != sc.end())
        servers = toset(*it);
    if ((it = sc.find("channels")) != sc.end())
        channels = toset(*it);
    if ((it = sc.find("origins")) != sc.end())
        origins = toset(*it);
    if ((it = sc.find("plugins")) != sc.end())
        plugins = toset(*it);
    if ((it = sc.find("channels")) != sc.end())
        channels = toset(*it);

    // Get the action.
    if ((it = sc.find("action")) == sc.end())
        throw std::invalid_argument("rule: missing 'action'' parameter");

    if (it->value() == "drop")
        action = rule::action_type::drop;
    else if (it->value() == "accept")
        action = rule::action_type::accept;
    else
        throw std::invalid_argument(string_util::sprintf("rule: invalid action given: %s", it->value()));

    return {
        std::move(servers),
        std::move(channels),
        std::move(origins),
        std::move(plugins),
        std::move(events),
        action
    };
}

std::shared_ptr<server> load_server(irccd& daemon, const ini::section& sc, const config& config)
{
    assert(sc.key() == "server");

    // Name.
    ini::section::const_iterator it;

    if ((it = sc.find("name")) == sc.end())
        throw std::invalid_argument("server: missing 'name' parameter");
    else if (!string_util::is_identifier(it->value()))
        throw std::invalid_argument(string_util::sprintf("server: invalid identifier: %s", it->value()));

    auto sv = std::make_shared<server>(daemon.service(), it->value());

    // Host
    if ((it = sc.find("host")) == sc.end())
        throw std::invalid_argument(string_util::sprintf("server %s: missing host", sv->name()));

    sv->set_host(it->value());

    // Optional password
    if ((it = sc.find("password")) != sc.end())
        sv->set_password(it->value());

    // Optional flags
    if ((it = sc.find("ipv6")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::ipv6);
    if ((it = sc.find("ssl")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::ssl);
    if ((it = sc.find("ssl-verify")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::ssl_verify);

    // Optional identity
    if ((it = sc.find("identity")) != sc.end())
        config.load_server_identity(*sv, it->value());

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
    try {
        if ((it = sc.find("port")) != sc.end())
            sv->set_port(string_util::to_uint<std::uint16_t>(it->value()));
        if ((it = sc.find("reconnect-tries")) != sc.end())
            sv->set_reconnect_tries(string_util::to_int<std::int8_t>(it->value()));
        if ((it = sc.find("reconnect-timeout")) != sc.end())
            sv->set_reconnect_delay(string_util::to_uint<std::uint16_t>(it->value()));
        if ((it = sc.find("ping-timeout")) != sc.end())
            sv->set_ping_timeout(string_util::to_uint<std::uint16_t>(it->value()));
    } catch (const std::exception&) {
        log::warning(string_util::sprintf("server %s: invalid number for %s: %s",
            sv->name(), it->key(), it->value()));
    }

    return sv;
}

} // !namespace

config config::find()
{
    for (const auto& path : sys::config_filenames("irccd.conf")) {
        try {
            boost::system::error_code ec;

            if (boost::filesystem::exists(path, ec) && !ec)
                return config(path);
        } catch (const std::exception& ex) {
            log::warning() << path << ": " << ex.what() << std::endl;
        }
    }

    throw std::runtime_error("no configuration file found");
}

void config::load_server_identity(server& server, const std::string& identity) const
{
    auto sc = std::find_if(document_.begin(), document_.end(), [&] (const auto& sc) {
        if (sc.key() != "identity")
            return false;

        auto name = sc.find("name");

        return name != sc.end() && name->value() == identity;
    });

    if (sc == document_.end())
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

bool config::is_verbose() const noexcept
{
    return string_util::is_boolean(get(document_, "logs", "verbose"));
}

bool config::is_foreground() const noexcept
{
    return string_util::is_boolean(get(document_, "general", "foreground"));
}

std::string config::pidfile() const
{
    return get(document_, "general", "pidfile");
}

std::string config::uid() const
{
    return get(document_, "general", "uid");
}

std::string config::gid() const
{
    return get(document_, "general", "gid");
}

void config::load_logs() const
{
    ini::document::const_iterator sc = document_.find("logs");

    if (sc == document_.end())
        return;

    ini::section::const_iterator it;

    if ((it = sc->find("type")) != sc->end()) {
        std::unique_ptr<log::logger> iface;

        // Console is the default, no test case.
        if (it->value() == "file")
            iface = load_log_file(*sc);
        else if (it->value() == "syslog")
            iface = load_log_syslog();
        else if (it->value() != "console")
            throw std::runtime_error(string_util::sprintf("logs: unknown log type: %s", it->value()));

        if (iface)
            log::set_logger(std::move(iface));
    }
}

void config::load_formats() const
{
    ini::document::const_iterator sc = document_.find("format");
    ini::section::const_iterator it;

    if (sc == document_.end())
        return;

    auto filter = std::make_unique<irccd_log_filter>();

    if ((it = sc->find("debug")) != sc->cend())
        filter->debug_ = it->value();
    if ((it = sc->find("info")) != sc->cend())
        filter->info_ = it->value();
    if ((it = sc->find("warning")) != sc->cend())
        filter->warning_ = it->value();

    log::set_filter(std::move(filter));
}

void config::load_transports(irccd& irccd) const
{
    for (const auto& section : document_)
        if (section.key() == "transport")
            irccd.transports().add(load_transport(irccd.service(), section));
}

std::vector<rule> config::load_rules() const
{
    std::vector<rule> rules;

    for (const auto& section : document_)
        if (section.key() == "rule")
            rules.push_back(load_rule(section));

    return rules;
}

std::vector<std::shared_ptr<server>> config::load_servers(irccd& daemon) const
{
    std::vector<std::shared_ptr<server>> servers;

    for (const auto& section : document_) {
        if (section.key() != "server")
            continue;

        try {
            servers.push_back(load_server(daemon, section, *this));
        } catch (const std::exception& ex) {
            log::warning(ex.what());
        }
    }

    return servers;
}

void config::load_plugins(irccd& irccd) const
{
    auto it = document_.find("plugins");

    if (it == document_.end())
        return;

    for (const auto& option : *it) {
        if (!string_util::is_identifier(option.key()))
            continue;

        irccd.plugins().load(option.key(), option.value());
    }
}

} // !irccd

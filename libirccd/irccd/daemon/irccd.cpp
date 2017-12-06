/*
 * irccd.cpp -- main irccd class
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

#include <irccd/logger.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include "command_service.hpp"
#include "irccd.hpp"
#include "plugin_service.hpp"
#include "rule_service.hpp"
#include "server_service.hpp"
#include "transport_service.hpp"

namespace irccd {

namespace {

class log_filter : public log::filter {
private:
    std::string info_;
    std::string warning_;
    std::string debug_;

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
    inline log_filter(std::string info, std::string warning, std::string debug) noexcept
        : info_(std::move(info))
        , warning_(std::move(warning))
        , debug_(std::move(debug))
    {
    }

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

void load_log_file(const ini::section& sc)
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

    try {
        log::set_logger(std::make_unique<log::file_logger>(std::move(normal), std::move(errors)));
    } catch (const std::exception& ex) {
        log::warning() << "logs: " << ex.what() << std::endl;
    }
}

void load_log_syslog()
{
#if defined(HAVE_SYSLOG)
    log::set_logger(std::make_unique<log::syslog_logger>());
#else
    log::warning() << "logs: syslog is not available on this platform" << std::endl;
#endif // !HAVE_SYSLOG
}

} // !namespace

void irccd::load_logs()
{
    auto sc = config_.section("logs");

    if (sc.empty())
        return;

    log::set_verbose(string_util::is_identifier(sc.get("verbose").value()));

    auto type = sc.get("type").value();

    if (!type.empty()) {
        // Console is the default, no test case.
        if (type == "file")
            load_log_file(sc);
        else if (type == "syslog")
            load_log_syslog();
        else if (type != "console")
            log::warning() << "logs: invalid log type '" << type << std::endl;
    }
}

void irccd::load_formats()
{
    auto sc = config_.section("format");

    if (sc.empty())
        return;

    log::set_filter(std::make_unique<log_filter>(
        sc.get("info").value(),
        sc.get("warning").value(),
        sc.get("debug").value()
    ));
}

void irccd::load_pid()
{
    auto path = config_.value("general", "pidfile");

    if (path.empty())
        return;

#if defined(HAVE_GETPID)
    std::ofstream out(path, std::ofstream::trunc);

    if (!out)
        log::warning() << "irccd: could not open" << path << ": " << std::strerror(errno) << std::endl;
    else {
        log::debug() << "irccd: pid written in " << path << std::endl;
        out << getpid() << std::endl;
    }
#else
    log::warning() << "irccd: pidfile not supported on this platform" << std::endl;
#endif
}

void irccd::load_gid()
{
    auto gid = config_.value("general", "gid");

    if (gid.empty())
        return;

#if defined(HAVE_SETGID)
    try {
        sys::set_gid(gid);
        log::info() << "irccd: setting gid to: " << gid << std::endl;
    } catch (const std::exception& ex) {
        log::warning() << "irccd: failed to set gid: " << ex.what() << std::endl;
    }
#else
    log::warning() << "irccd: gid option not supported" << std::endl;
#endif
}

void irccd::load_uid()
{
    auto uid = config_.value("general", "uid");

    if (uid.empty())
        return;

#if defined(HAVE_SETUID)
    try {
        sys::set_uid(uid);
        log::info() << "irccd: setting uid to: " << uid << std::endl;
    } catch (const std::exception& ex) {
        log::warning() << "irccd: failed to set uid: " << ex.what() << std::endl;
    }
#else
    log::warning() << "irccd: uid option not supported" << std::endl;
#endif
}

irccd::irccd(boost::asio::io_service& service, std::string config)
    : config_(std::move(config))
    , service_(service)
    , command_service_(std::make_unique<command_service>())
    , server_service_(std::make_unique<server_service>(*this))
    , tpt_service_(std::make_unique<transport_service>(*this))
    , rule_service_(std::make_unique<rule_service>())
    , plugin_service_(std::make_unique<plugin_service>(*this))
{
}

irccd::~irccd() = default;

void irccd::load() noexcept
{
    /*
     * Order matters, please be careful when changing this.
     *
     * 1. Open logs as early as possible to use the defined outputs on any
     *    loading errors.
     */

    // [logs] and [format] sections.
    load_logs();
    load_formats();

    if (!loaded_)
        log::info() << "irccd: loading configuration from " << config_.path() << std::endl;
    else
        log::info() << "irccd: reloading configuration" << std::endl;

    // [general] section.
    if (!loaded_) {
        load_pid();
        load_gid();
        load_uid();
    }

    if (!loaded_)
        tpt_service_->load(config_);

    server_service_->load(config_);
    plugin_service_->load(config_);
    rule_service_->load(config_);

    // Mark as loaded.
    loaded_ = true;
}

const boost::system::error_category& irccd_category()
{
    static const class category : public boost::system::error_category {
    public:
        const char* name() const noexcept override
        {
            return "irccd";
        }

        std::string message(int e) const override
        {
            switch (static_cast<irccd_error::error>(e)) {
            case irccd_error::error::not_irccd:
                return "daemon is not irccd instance";
            case irccd_error::error::incompatible_version:
                return "major version is incompatible";
            case irccd_error::error::auth_required:
                return "authentication is required";
            case irccd_error::error::invalid_auth:
                return "invalid authentication";
            case irccd_error::error::invalid_message:
                return "invalid message";
            case irccd_error::error::invalid_command:
                return "invalid command";
            case irccd_error::error::incomplete_message:
                return "command requires more arguments";
            default:
                return "no error";
            }
        }
    } category;

    return category;
}

boost::system::error_code make_error_code(irccd_error::error e)
{
    return {static_cast<int>(e), irccd_category()};
}

} // !irccd

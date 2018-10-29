/*
 * irccd.cpp -- main irccd class
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

#include <fstream>

#include <boost/predef/os.h>

#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include "irccd.hpp"
#include "logger.hpp"
#include "plugin_service.hpp"
#include "rule_service.hpp"
#include "server_service.hpp"
#include "transport_service.hpp"

namespace irccd {

namespace {

class format_filter : public logger::filter {
private:
	std::string info_;
	std::string warning_;
	std::string debug_;

	auto convert(const std::string&,
	             std::string_view,
	             std::string_view,
	             std::string_view) const -> std::string;

public:
	format_filter(std::string info, std::string warning, std::string debug) noexcept;

	auto pre_debug(std::string_view,
	               std::string_view,
	               std::string_view) const -> std::string override;

	auto pre_info(std::string_view,
	              std::string_view,
	              std::string_view) const -> std::string override;

	auto pre_warning(std::string_view,
	                 std::string_view,
	                 std::string_view) const -> std::string override;
};

auto format_filter::convert(const std::string& tmpl,
                            std::string_view category,
                            std::string_view component,
                            std::string_view message) const -> std::string
{
	if (tmpl.empty())
		return pre(category, component, message);

	string_util::subst params;

	params.flags &= ~(string_util::subst_flags::irc_attrs);
	params.flags |= string_util::subst_flags::shell_attrs;
	params.keywords.emplace("category", std::string(category));
	params.keywords.emplace("component", std::string(component));
	params.keywords.emplace("message", std::string(message));

	return string_util::format(tmpl, params);
}

format_filter::format_filter(std::string info, std::string warning, std::string debug) noexcept
	: info_(std::move(info))
	, warning_(std::move(warning))
	, debug_(std::move(debug))
{
}

auto format_filter::pre_debug(std::string_view category,
                              std::string_view component,
                              std::string_view message) const -> std::string
{
	return convert(debug_, category, component, message);
}

auto format_filter::pre_info(std::string_view category,
                             std::string_view component,
                             std::string_view message) const -> std::string
{
	return convert(info_, category, component, message);
}

auto format_filter::pre_warning(std::string_view category,
                                std::string_view component,
                                std::string_view message) const -> std::string
{
	return convert(warning_, category, component, message);
}

} // !namespace

void irccd::load_logs_file(const ini::section& sc)
{
	/*
	 * TODO: improve that with CMake options.
	 */
#if BOOST_OS_WINDOWS
	std::string normal = "log.txt";
	std::string errors = "errors.txt";
#else
	std::string normal = "/var/log/irccd/log.txt";
	std::string errors = "/var/log/irccd/errors.txt";
#endif

	ini::section::const_iterator it;

	if ((it = sc.find("path-logs")) != sc.end())
		normal = it->get_value();
	if ((it = sc.find("path-errors")) != sc.end())
		errors = it->get_value();

	try {
		sink_ = std::make_unique<logger::file_sink>(std::move(normal), std::move(errors));
	} catch (const std::exception& ex) {
		sink_->warning("logs", "") << ex.what() << std::endl;
	}
}

void irccd::load_logs_syslog()
{
#if defined(IRCCD_HAVE_SYSLOG)
	sink_ = std::make_unique<logger::syslog_sink>();
#else
	sink_->warning("logs", "") << "logs: syslog is not available on this platform" << std::endl;
#endif // !IRCCD_HAVE_SYSLOG
}

void irccd::load_logs()
{
	const auto sc = config_.get("logs");

	if (sc.empty())
		return;

	sink_->set_verbose(string_util::is_identifier(sc.get("verbose").get_value()));

	const auto type = sc.get("type").get_value();

	if (!type.empty()) {
		// Console is the default, no test case.
		if (type == "file")
			load_logs_file(sc);
		else if (type == "syslog")
			load_logs_syslog();
		else if (type != "console")
			sink_->warning("logs", "") << "invalid log type '" << type << std::endl;
	}
}

void irccd::load_formats()
{
	const auto sc = config_.get("format");

	if (sc.empty())
		return;

	sink_->set_filter(std::make_unique<format_filter>(
		sc.get("info").get_value(),
		sc.get("warning").get_value(),
		sc.get("debug").get_value()
	));
}

irccd::irccd(boost::asio::io_service& service, std::string config)
	: config_(std::move(config))
	, service_(service)
	, sink_(std::make_unique<logger::console_sink>())
	, server_service_(std::make_unique<server_service>(*this))
	, tpt_service_(std::make_unique<transport_service>(*this))
	, rule_service_(std::make_unique<rule_service>(*this))
	, plugin_service_(std::make_unique<plugin_service>(*this))
{
}

irccd::~irccd() = default;

auto irccd::get_config() const noexcept -> const config&
{
	return config_;
}

void irccd::set_config(config cfg) noexcept
{
	config_ = std::move(cfg);
}

auto irccd::get_service() const noexcept -> const boost::asio::io_service&
{
	return service_;
}

auto irccd::get_service() noexcept -> boost::asio::io_service&
{
	return service_;
}

auto irccd::get_log() const noexcept -> const logger::sink&
{
	return *sink_;
}

auto irccd::get_log() noexcept -> logger::sink&
{
	return *sink_;
}

auto irccd::servers() noexcept -> server_service&
{
	return *server_service_;
}

auto irccd::transports() noexcept -> transport_service&
{
	return *tpt_service_;
}

auto irccd::rules() noexcept -> rule_service&
{
	return *rule_service_;
}

auto irccd::plugins() noexcept -> plugin_service&
{
	return *plugin_service_;
}

void irccd::set_log(std::unique_ptr<logger::sink> sink) noexcept
{
	assert(sink);

	sink_ = std::move(sink);
}

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
		sink_->info("irccd", "") << "loading configuration from " << config_.get_path() << std::endl;
	else
		sink_->info("irccd", "") << "reloading configuration" << std::endl;

	if (!loaded_)
		tpt_service_->load(config_);

	server_service_->load(config_);
	plugin_service_->load(config_);
	rule_service_->load(config_);

	// Mark as loaded.
	loaded_ = true;
}

auto irccd_category() noexcept -> const std::error_category&
{
	static const class category : public std::error_category {
	public:
		auto name() const noexcept -> const char* override
		{
			return "irccd";
		}

		auto message(int e) const -> std::string override
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

auto make_error_code(irccd_error::error e) noexcept -> std::error_code
{
	return { static_cast<int>(e), irccd_category() };
}

} // !irccd

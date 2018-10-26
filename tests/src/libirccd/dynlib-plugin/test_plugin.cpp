/*
 * test_plugin.cpp -- basic exported plugin test
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

#include <irccd/daemon/plugin.hpp>

namespace irccd {

class test_plugin : public plugin {
private:
	map config_;

public:
	test_plugin()
		: plugin("test")
	{
	}

	auto get_options() const -> map override
	{
		return config_;
	}

	auto get_name() const noexcept -> std::string_view override
	{
		return "test";
	}

	void handle_command(irccd&, const message_event&) override
	{
		config_["command"] = "true";
	}

	void handle_connect(irccd&, const connect_event&) override
	{
		config_["connect"] = "true";
	}

	void handle_invite(irccd&, const invite_event&) override
	{
		config_["invite"] = "true";
	}

	void handle_join(irccd&, const join_event&) override
	{
		config_["join"] = "true";
	}

	void handle_kick(irccd&, const kick_event&) override
	{
		config_["kick"] = "true";
	}

	void handle_load(irccd&) override
	{
		config_["load"] = "true";
	}

	void handle_message(irccd&, const message_event&) override
	{
		config_["message"] = "true";
	}

	void handle_me(irccd&, const me_event&) override
	{
		config_["me"] = "true";
	}

	void handle_mode(irccd&, const mode_event&) override
	{
		config_["mode"] = "true";
	}

	void handle_names(irccd&, const names_event&) override
	{
		config_["names"] = "true";
	}

	void handle_nick(irccd&, const nick_event&) override
	{
		config_["nick"] = "true";
	}

	void handle_notice(irccd&, const notice_event&) override
	{
		config_["notice"] = "true";
	}

	void handle_part(irccd&, const part_event&) override
	{
		config_["part"] = "true";
	}

	void handle_reload(irccd&) override
	{
		config_["reload"] = "true";
	}

	void handle_topic(irccd&, const topic_event&) override
	{
		config_["topic"] = "true";
	}

	void handle_unload(irccd&) override
	{
		config_["unload"] = "true";
	}

	void handle_whois(irccd&, const whois_event&) override
	{
		config_["whois"] = "true";
	}

	static auto abi() -> version
	{
		return version();
	}

	static auto init(std::string) -> std::unique_ptr<plugin>
	{
		return std::make_unique<test_plugin>();
	}
};

BOOST_DLL_ALIAS(test_plugin::abi, irccd_abi_test_plugin)
BOOST_DLL_ALIAS(test_plugin::init, irccd_init_test_plugin)

} // !irccd

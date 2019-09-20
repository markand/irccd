/*
 * main.cpp -- test hook functions
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "hook"
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/hook.hpp>
#include <irccd/daemon/logger.hpp>

#include <irccd/test/mock_server.hpp>

using std::string;
using std::vector;
using std::shared_ptr;
using std::make_unique;
using std::make_shared;

using boost::asio::io_context;

using irccd::daemon::logger::sink;

using irccd::test::mock_server;

namespace irccd::daemon {

namespace {

/*
 * Since stdout/stderr from the hook is logger through the irccd's logger, we'll
 * gonna store every message logged into it and compare if the values are
 * appropriate.
 */
class memory_sink : public logger::sink {
public:
	using list = vector<string>;

private:
	list warning_;
	list info_;

public:
	auto get_info() const noexcept -> const list&;
	auto get_warning() const noexcept -> const list&;
	void write_debug(const std::string& line) override;
	void write_info(const std::string& line) override;
	void write_warning(const std::string& line) override;
};

auto memory_sink::get_info() const noexcept -> const list&
{
	return info_;
}

auto memory_sink::get_warning() const noexcept -> const list&
{
	return warning_;
}

void memory_sink::write_debug(const std::string&)
{
}

void memory_sink::write_info(const std::string& line)
{
	info_.push_back(line);
}

void memory_sink::write_warning(const std::string& line)
{
	warning_.push_back(line);
}

class hook_fixture {
protected:
	io_context io_;
	bot bot_{io_};
	hook hook_{"test", HOOK_FILE};
	memory_sink* sink_{nullptr};
	shared_ptr<mock_server> server_;

public:
	hook_fixture();
};

hook_fixture::hook_fixture()
	: server_(make_shared<mock_server>(io_, "test"))
{
	auto sink = make_unique<memory_sink>();

	sink_ = sink.get();
	bot_.set_log(std::move(sink));
	bot_.get_log().set_verbose(true);
}

BOOST_FIXTURE_TEST_SUITE(hook_fixture_suite, hook_fixture)

BOOST_AUTO_TEST_CASE(connect)
{
	hook_.handle_connect(bot_, {server_});

	BOOST_TEST(sink_->get_info().size() == 2U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onConnect");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
}

BOOST_AUTO_TEST_CASE(disconnect)
{
	hook_.handle_disconnect(bot_, {server_});

	BOOST_TEST(sink_->get_info().size() == 2U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onDisconnect");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
}

BOOST_AUTO_TEST_CASE(invite)
{
	hook_.handle_invite(bot_, {server_, "jean", "#staff", "NiReaS"});

	BOOST_TEST(sink_->get_info().size() == 5U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onInvite");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #staff");
	BOOST_TEST(sink_->get_info()[4] == "hook test: target:  NiReaS");
}

BOOST_AUTO_TEST_CASE(join)
{
	hook_.handle_join(bot_, {server_, "jean", "#staff"});

	BOOST_TEST(sink_->get_info().size() == 4U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onJoin");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #staff");
}

BOOST_AUTO_TEST_CASE(kick)
{
	hook_.handle_kick(bot_, {server_, "jean", "#staff", "NiReaS", "stop it"});

	BOOST_TEST(sink_->get_info().size() == 6U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onKick");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #staff");
	BOOST_TEST(sink_->get_info()[4] == "hook test: target:  NiReaS");
	BOOST_TEST(sink_->get_info()[5] == "hook test: reason:  stop it");
}

BOOST_AUTO_TEST_CASE(message)
{
	hook_.handle_message(bot_, {server_, "jean", "#staff", "coucou tout le monde"});

	BOOST_TEST(sink_->get_info().size() == 5U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onMessage");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #staff");
	BOOST_TEST(sink_->get_info()[4] == "hook test: message: coucou tout le monde");
}

BOOST_AUTO_TEST_CASE(me)
{
	hook_.handle_me(bot_, {server_, "jean", "#staff", "coucou tout le monde"});

	BOOST_TEST(sink_->get_info().size() == 5U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onMe");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #staff");
	BOOST_TEST(sink_->get_info()[4] == "hook test: message: coucou tout le monde");
}

BOOST_AUTO_TEST_CASE(mode)
{
	hook_.handle_mode(bot_, {server_, "jean", "#staff", "+o", "franck", "abc", "xyz" });

	BOOST_TEST(sink_->get_info().size() == 8U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onMode");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #staff");
	BOOST_TEST(sink_->get_info()[4] == "hook test: mode:    +o");
	BOOST_TEST(sink_->get_info()[5] == "hook test: limit:   franck");
	BOOST_TEST(sink_->get_info()[6] == "hook test: user:    abc");
	BOOST_TEST(sink_->get_info()[7] == "hook test: mask:    xyz");
}

BOOST_AUTO_TEST_CASE(nick)
{
	hook_.handle_nick(bot_, {server_, "jean", "doctor"});

	BOOST_TEST(sink_->get_info().size() == 4U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onNick");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: nick:    doctor");
}

BOOST_AUTO_TEST_CASE(notice)
{
	hook_.handle_notice(bot_, {server_, "jean", "#staff", "coucou tout le monde"});

	BOOST_TEST(sink_->get_info().size() == 5U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onNotice");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #staff");
	BOOST_TEST(sink_->get_info()[4] == "hook test: message: coucou tout le monde");
}

BOOST_AUTO_TEST_CASE(part)
{
	hook_.handle_part(bot_, {server_, "jean", "#windows", "je n'aime pas ici"});

	BOOST_TEST(sink_->get_info().size() == 5U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onPart");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #windows");
	BOOST_TEST(sink_->get_info()[4] == "hook test: reason:  je n'aime pas ici");
}

BOOST_AUTO_TEST_CASE(topic)
{
	hook_.handle_topic(bot_, {server_, "jean", "#windows", "attention Windows est un malware"});

	BOOST_TEST(sink_->get_info().size() == 5U);
	BOOST_TEST(sink_->get_warning().empty());
	BOOST_TEST(sink_->get_info()[0] == "hook test: event:   onTopic");
	BOOST_TEST(sink_->get_info()[1] == "hook test: server:  test");
	BOOST_TEST(sink_->get_info()[2] == "hook test: origin:  jean");
	BOOST_TEST(sink_->get_info()[3] == "hook test: channel: #windows");
	BOOST_TEST(sink_->get_info()[4] == "hook test: topic:   attention Windows est un malware");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd::daemon

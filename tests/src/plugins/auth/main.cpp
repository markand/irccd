/*
 * main.cpp -- test auth plugin
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

#define BOOST_TEST_MODULE "Auth plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server.hpp>

#include <irccd/test/js_plugin_fixture.hpp>

namespace irccd {

namespace {

class auth_test : public js_plugin_fixture {
protected:
	std::shared_ptr<mock_server> nickserv1_;
	std::shared_ptr<mock_server> nickserv2_;
	std::shared_ptr<mock_server> quakenet_;

public:
	auth_test()
		: js_plugin_fixture(PLUGIN_PATH)
		, nickserv1_(std::make_shared<mock_server>(service_, "nickserv1", "localhost"))
		, nickserv2_(std::make_shared<mock_server>(service_, "nickserv2", "localhost"))
		, quakenet_(std::make_shared<mock_server>(service_, "quakenet", "localhost"))
	{
		plugin_->set_options({
			{ "nickserv1.type",     "nickserv"      },
			{ "nickserv1.password", "plopation"     },
			{ "nickserv2.type",     "nickserv"      },
			{ "nickserv2.password", "something"     },
			{ "nickserv2.username", "jean"          },
			{ "quakenet.type",      "quakenet"      },
			{ "quakenet.password",  "hello"         },
			{ "quakenet.username",  "mario"         }
		});
		plugin_->handle_load(irccd_);
	}
};

BOOST_FIXTURE_TEST_SUITE(auth_test_suite, auth_test)

BOOST_AUTO_TEST_CASE(nickserv1)
{
	plugin_->handle_connect(irccd_, { nickserv1_ });

	const auto cmd = nickserv1_->find("message").front();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "NickServ");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "identify plopation");
}

BOOST_AUTO_TEST_CASE(nickserv2)
{
	plugin_->handle_connect(irccd_, { nickserv2_ });

	const auto cmd = nickserv2_->find("message").front();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "NickServ");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "identify jean something");
}

BOOST_AUTO_TEST_CASE(quakenet)
{
	plugin_->handle_connect(irccd_, { quakenet_ });

	const auto cmd = quakenet_->find("message").front();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "Q@CServe.quakenet.org");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "AUTH mario hello");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd

/*
 * main.cpp -- test Command class
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <gtest/gtest.h>

#include <irccd/command.hpp>

using namespace irccd;

class MyCommand : public Command {
public:
	MyCommand()
		: Command("test", "Test")
	{
	}

	std::string help() const override
	{
		return "This is a super command";
	}

	std::vector<Property> properties() const
	{
		return {
			{ "b", { json::Type::Boolean			} },
			{ "i", { json::Type::Int			} },
			{ "m", { json::Type::Boolean, json::Type::Int, json::Type::String	} }
		};
	}
};

TEST(Properties, valid)
{
	Irccd *irccd = nullptr;
	MyCommand cmd;

	ASSERT_NO_THROW(cmd.exec(*irccd, json::object({
		{ "b", true	},
		{ "i", 123	},
		{ "m", "abc"	}
	})));

	ASSERT_NO_THROW(cmd.exec(*irccd, json::object({
		{ "b", true	},
		{ "i", 123	},
		{ "m", 456	}
	})));

	ASSERT_NO_THROW(cmd.exec(*irccd, json::object({
		{ "b", true	},
		{ "i", 123	},
		{ "m", "456"	}
	})));
}

TEST(Properties, missingB)
{
	Irccd *irccd = nullptr;
	MyCommand cmd;

	ASSERT_THROW(cmd.exec(*irccd, json::object({
		{ "i", 123	},
		{ "m", "abc"	}
	})), std::invalid_argument);
}

TEST(Properties, missingI)
{
	Irccd *irccd = nullptr;
	MyCommand cmd;

	ASSERT_THROW(cmd.exec(*irccd, json::object({
		{ "b", true	},
		{ "m", "abc"	}
	})), std::invalid_argument);
}

TEST(Properties, missingM)
{
	Irccd *irccd = nullptr;
	MyCommand cmd;

	ASSERT_THROW(cmd.exec(*irccd, json::object({
		{ "b", true	},
		{ "i", 123	},
	})), std::invalid_argument);
}

TEST(Properties, invalidB)
{
	Irccd *irccd = nullptr;
	MyCommand cmd;

	ASSERT_THROW(cmd.exec(*irccd, json::object({
		{ "b", "fail"	},
		{ "i", 123	},
		{ "m", "abc"	}
	})), std::invalid_argument);
}

TEST(Properties, invalidM)
{
	Irccd *irccd = nullptr;
	MyCommand cmd;

	ASSERT_THROW(cmd.exec(*irccd, json::object({
		{ "b", "fail"	},
		{ "i", 123	},
		{ "m", nullptr	}
	})), std::invalid_argument);
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
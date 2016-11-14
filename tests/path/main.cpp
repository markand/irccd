/*
 * main.cpp -- test path functions
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

#include <irccd/sysconfig.hpp>
#include <irccd/logger.hpp>
#include <irccd/path.hpp>

namespace irccd {

/* --------------------------------------------------------
 * Back slashes
 * -------------------------------------------------------- */

#if defined(IRCCD_SYSTEM_WINDOWS)

TEST(Back, nochange)
{
    std::string path = "\\usr\\local\\etc\\";
    std::string result = path::clean(path);

    ASSERT_EQ(path, result);
}

TEST(Back, duplicateBegin)
{
    std::string path = "\\\\usr\\local\\etc\\";
    std::string result = path::clean(path);

    ASSERT_EQ("\\usr\\local\\etc\\", result);
}

TEST(Back, duplicateEnd)
{
    std::string path = "\\usr\\local\\etc\\\\";
    std::string result = path::clean(path);

    ASSERT_EQ("\\usr\\local\\etc\\", result);
}

TEST(Back, duplicateEverywhere)
{
    std::string path = "\\\\usr\\\\local\\\\etc\\\\";
    std::string result = path::clean(path);

    ASSERT_EQ("\\usr\\local\\etc\\", result);
}

TEST(Back, missingTrailing)
{
    std::string path = "\\usr\\local\\etc";
    std::string result = path::clean(path);

    ASSERT_EQ("\\usr\\local\\etc\\", result);
}

#else

/* --------------------------------------------------------
 * Forward slashes
 * -------------------------------------------------------- */

TEST(Forward, nochange)
{
    std::string path = "/usr/local/etc/";
    std::string result = path::clean(path);

    ASSERT_EQ(path, result);
}

TEST(Forward, duplicateBegin)
{
    std::string path = "//usr/local/etc/";
    std::string result = path::clean(path);

    ASSERT_EQ("/usr/local/etc/", result);
}

TEST(Forward, duplicateEnd)
{
    std::string path = "/usr/local/etc//";
    std::string result = path::clean(path);

    ASSERT_EQ("/usr/local/etc/", result);
}

TEST(Forward, duplicateEverywhere)
{
    std::string path = "//usr//local//etc//";
    std::string result = path::clean(path);

    ASSERT_EQ("/usr/local/etc/", result);
}

TEST(Forward, missingTrailing)
{
    std::string path = "/usr/local/etc";
    std::string result = path::clean(path);

    ASSERT_EQ("/usr/local/etc/", result);
}

#endif

} // !irccd

using namespace irccd;

int main(int argc, char **argv)
{
    /*
     * Just show everything for test purpose.
     */
    path::setApplicationPath(argv[0]);
    log::debug() << "System paths:" << std::endl;
    log::debug() << "  config(system):  " << path::get(path::PathConfig, path::OwnerSystem) << std::endl;
    log::debug() << "  data(system):    " << path::get(path::PathData, path::OwnerSystem) << std::endl;
    log::debug() << "  plugins(system): " << path::get(path::PathPlugins, path::OwnerSystem) << std::endl;
    log::debug() << "  cache(system):   " << path::get(path::PathCache, path::OwnerSystem) << std::endl;
    log::debug() << "User paths:" << std::endl;
    log::debug() << "  config(user):    " << path::get(path::PathConfig, path::OwnerUser) << std::endl;
    log::debug() << "  data(user):      " << path::get(path::PathData, path::OwnerUser) << std::endl;
    log::debug() << "  plugins(user):   " << path::get(path::PathPlugins, path::OwnerUser) << std::endl;
    log::debug() << "  cache(user):     " << path::get(path::PathCache, path::OwnerUser) << std::endl;

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

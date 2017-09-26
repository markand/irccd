/*
 * main.cpp -- test irccd rules
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

#include <gtest/gtest.h>

#include <irccd/irccd.hpp>
#include <irccd/service.hpp>

namespace irccd {

TEST(service_plugin, default_paths)
{
    irccd irccd;

    irccd.plugins().set_paths({
        { "cache",  "/var/cache/irccd"          },
        { "config", "/etc/irccd"                },
        { "data",   "/usr/local/share/irccd"    }
    });

    auto paths = irccd.plugins().paths("ask");

    ASSERT_EQ("/var/cache/irccd/plugin/ask", paths["cache"]);
    ASSERT_EQ("/etc/irccd/plugin/ask", paths["config"]);
    ASSERT_EQ("/usr/local/share/irccd/plugin/ask", paths["data"]);
}

TEST(service_plugin, override_cache)
{
    irccd irccd;

    irccd.plugins().set_paths({
        { "cache",  "/var/cache/irccd"          },
        { "config", "/etc/irccd"                },
        { "data",   "/usr/local/share/irccd"    }
    });
    irccd.plugins().set_paths("ask", {
        { "cache",  "/opt/cache/ask"            }
    });

    auto paths = irccd.plugins().paths("ask");

    ASSERT_EQ("/opt/cache/ask", paths["cache"]);
    ASSERT_EQ("/etc/irccd/plugin/ask", paths["config"]);
    ASSERT_EQ("/usr/local/share/irccd/plugin/ask", paths["data"]);
}

TEST(service_plugin, override_config)
{
    irccd irccd;

    irccd.plugins().set_paths({
        { "cache",  "/var/cache/irccd"          },
        { "config", "/etc/irccd"                },
        { "data",   "/usr/local/share/irccd"    }
    });
    irccd.plugins().set_paths("ask", {
        { "config", "/opt/config/ask"           }
    });

    auto paths = irccd.plugins().paths("ask");

    ASSERT_EQ("/var/cache/irccd/plugin/ask", paths["cache"]);
    ASSERT_EQ("/opt/config/ask", paths["config"]);
    ASSERT_EQ("/usr/local/share/irccd/plugin/ask", paths["data"]);
}

TEST(service_plugin, override_data)
{
    irccd irccd;

    irccd.plugins().set_paths({
        { "cache",  "/var/cache/irccd"          },
        { "config", "/etc/irccd"                },
        { "data",   "/usr/local/share/irccd"    }
    });
    irccd.plugins().set_paths("ask", {
        { "data",   "/opt/data/ask"             }
    });

    auto paths = irccd.plugins().paths("ask");

    ASSERT_EQ("/var/cache/irccd/plugin/ask", paths["cache"]);
    ASSERT_EQ("/etc/irccd/plugin/ask", paths["config"]);
    ASSERT_EQ("/opt/data/ask", paths["data"]);
}

TEST(service_plugin, override_all)
{
    irccd irccd;

    irccd.plugins().set_paths({
        { "cache",  "/var/cache/irccd"          },
        { "config", "/etc/irccd"                },
        { "data",   "/usr/local/share/irccd"    }
    });
    irccd.plugins().set_paths("ask", {
        { "cache",  "/opt/cache/ask"            },
        { "config", "/opt/config/ask"           },
        { "data",   "/opt/data/ask"             }
    });

    auto paths = irccd.plugins().paths("ask");

    ASSERT_EQ("/opt/cache/ask", paths["cache"]);
    ASSERT_EQ("/opt/config/ask", paths["config"]);
    ASSERT_EQ("/opt/data/ask", paths["data"]);
}

TEST(service_plugin, extra_paths)
{
    irccd irccd;

    irccd.plugins().set_paths({
        { "cache",  "/var/cache/irccd"          },
        { "config", "/etc/irccd"                },
        { "data",   "/usr/local/share/irccd"    }
    });
    irccd.plugins().set_paths("ask", {
        { "extra",  "/opt/magic"                }
    });

    auto paths = irccd.plugins().paths("ask");

    ASSERT_EQ("/var/cache/irccd/plugin/ask", paths["cache"]);
    ASSERT_EQ("/etc/irccd/plugin/ask", paths["config"]);
    ASSERT_EQ("/usr/local/share/irccd/plugin/ask", paths["data"]);
    ASSERT_EQ("/opt/magic", paths["extra"]);
}

} // !irccd

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

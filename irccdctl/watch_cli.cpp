/*
 * watch_cli.cpp -- implementation of irccdctl watch
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

#include <functional>
#include <unordered_map>

#include <irccd/json_util.hpp>
#include <irccd/options.hpp>

#include <irccd/ctl/controller.hpp>

#include <boost/system/system_error.hpp>

#include "watch_cli.hpp"

namespace irccd {

namespace ctl {

namespace {

std::string format(std::vector<std::string> args)
{
    auto result = option::read(args, {
        { "-f",         true },
        { "--format",   true }
    });

    if (result.count("-f") > 0)
        return result.find("-f")->second;
    if (result.count("--format") > 0)
        return result.find("--format")->second;

    return "native";
}

void onChannelMode(const nlohmann::json &v)
{
    std::cout << "event:       onChannelMode\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "mode:        " << json_util::pretty(v, "mode") << "\n";
    std::cout << "argument:    " << json_util::pretty(v, "argument") << "\n";
}

void onChannelNotice(const nlohmann::json &v)
{
    std::cout << "event:       onChannelNotice\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onConnect(const nlohmann::json &v)
{
    std::cout << "event:       onConnect\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
}

void onInvite(const nlohmann::json &v)
{
    std::cout << "event:       onInvite\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
}

void onJoin(const nlohmann::json &v)
{
    std::cout << "event:       onJoin\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
}

void onKick(const nlohmann::json &v)
{
    std::cout << "event:       onKick\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "target:      " << json_util::pretty(v, "target") << "\n";
    std::cout << "reason:      " << json_util::pretty(v, "reason") << "\n";
}

void onMessage(const nlohmann::json &v)
{
    std::cout << "event:       onMessage\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onMe(const nlohmann::json &v)
{
    std::cout << "event:       onMe\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "target:      " << json_util::pretty(v, "target") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onMode(const nlohmann::json &v)
{
    std::cout << "event:       onMode\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "mode:        " << json_util::pretty(v, "mode") << "\n";
}

void onNames(const nlohmann::json &v)
{
    std::cout << "event:       onNames\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "names:       " << json_util::pretty(v, "names") << "\n";
}

void onNick(const nlohmann::json &v)
{
    std::cout << "event:       onNick\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "nickname:    " << json_util::pretty(v, "nickname") << "\n";
}

void onNotice(const nlohmann::json &v)
{
    std::cout << "event:       onNotice\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onPart(const nlohmann::json &v)
{
    std::cout << "event:       onPart\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "reason:      " << json_util::pretty(v, "reason") << "\n";
}

void onQuery(const nlohmann::json &v)
{
    std::cout << "event:       onQuery\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onTopic(const nlohmann::json &v)
{
    std::cout << "event:       onTopic\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "topic:       " << json_util::pretty(v, "topic") << "\n";
}

void onWhois(const nlohmann::json &v)
{
    std::cout << "event:       onWhois\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "nickname:    " << json_util::pretty(v, "nickname") << "\n";
    std::cout << "username:    " << json_util::pretty(v, "username") << "\n";
    std::cout << "host:        " << json_util::pretty(v, "host") << "\n";
    std::cout << "realname:    " << json_util::pretty(v, "realname") << "\n";
}

const std::unordered_map<std::string, std::function<void (const nlohmann::json&)>> events{
    { "onChannelMode",      onChannelMode   },
    { "onChannelNotice",    onChannelNotice },
    { "onConnect",          onConnect       },
    { "onInvite",           onInvite        },
    { "onJoin",             onJoin          },
    { "onKick",             onKick          },
    { "onMessage",          onMessage       },
    { "onMe",               onMe            },
    { "onMode",             onMode          },
    { "onNames",            onNames         },
    { "onNick",             onNick          },
    { "onNotice",           onNotice        },
    { "onPart",             onPart          },
    { "onQuery",            onQuery         },
    { "onTopic",            onTopic         },
    { "onWhois",            onWhois         }
};

void get_event(ctl::controller& ctl, std::string fmt)
{
    ctl.recv([&ctl, fmt] (auto code, auto message) {
        if (code)
            throw boost::system::system_error(code);

        auto it = events.find(json_util::to_string(message["event"]));

        if (it != events.end()) {
            if (fmt == "json")
                std::cout << message.dump(4) << std::endl;
            else {
                it->second(message);
                std::cout << std::endl;
            }
        }

        get_event(ctl, std::move(fmt));
    });
}

} // !namespace

std::string watch_cli::name() const
{
    return "watch";
}

void watch_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    auto fmt = format(args);

    if (fmt != "native" && fmt != "json")
        throw std::invalid_argument("invalid format given: " + fmt);

    get_event(ctl, fmt);
}

} // !ctl

} // !irccd

/*
 * watch_cli.cpp -- implementation of irccdctl watch
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

void onConnect(const nlohmann::json &v)
{
    std::cout << "event:       onConnect\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
}

void onInvite(const nlohmann::json &v)
{
    std::cout << "event:       onInvite\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "channel:     " << json_util::pretty(v.value("channel", "(unknown)")) << "\n";
}

void onJoin(const nlohmann::json &v)
{
    std::cout << "event:       onJoin\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "channel:     " << json_util::pretty(v.value("channel", "(unknown)")) << "\n";
}

void onKick(const nlohmann::json &v)
{
    std::cout << "event:       onKick\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "channel:     " << json_util::pretty(v.value("channel", "(unknown)")) << "\n";
    std::cout << "target:      " << json_util::pretty(v.value("target", "(unknown)")) << "\n";
    std::cout << "reason:      " << json_util::pretty(v.value("reason", "(unknown)")) << "\n";
}

void onMessage(const nlohmann::json &v)
{
    std::cout << "event:       onMessage\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "channel:     " << json_util::pretty(v.value("channel", "(unknown)")) << "\n";
    std::cout << "message:     " << json_util::pretty(v.value("message", "(unknown)")) << "\n";
}

void onMe(const nlohmann::json &v)
{
    std::cout << "event:       onMe\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "target:      " << json_util::pretty(v.value("target", "(unknown)")) << "\n";
    std::cout << "message:     " << json_util::pretty(v.value("message", "(unknown)")) << "\n";
}

void onMode(const nlohmann::json &v)
{
    std::cout << "event:       onMode\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "mode:        " << json_util::pretty(v.value("mode", "(unknown)")) << "\n";
}

void onNames(const nlohmann::json &v)
{
    std::cout << "event:       onNames\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "channel:     " << json_util::pretty(v.value("channel", "(unknown)")) << "\n";
    std::cout << "names:       " << json_util::pretty(v.value("names", "(unknown)")) << "\n";
}

void onNick(const nlohmann::json &v)
{
    std::cout << "event:       onNick\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "nickname:    " << json_util::pretty(v.value("nickname", "(unknown)")) << "\n";
}

void onNotice(const nlohmann::json &v)
{
    std::cout << "event:       onNotice\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "message:     " << json_util::pretty(v.value("message", "(unknown)")) << "\n";
}

void onPart(const nlohmann::json &v)
{
    std::cout << "event:       onPart\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "channel:     " << json_util::pretty(v.value("channel", "(unknown)")) << "\n";
    std::cout << "reason:      " << json_util::pretty(v.value("reason", "(unknown)")) << "\n";
}

void onTopic(const nlohmann::json &v)
{
    std::cout << "event:       onTopic\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "origin:      " << json_util::pretty(v.value("origin", "(unknown)")) << "\n";
    std::cout << "channel:     " << json_util::pretty(v.value("channel", "(unknown)")) << "\n";
    std::cout << "topic:       " << json_util::pretty(v.value("topic", "(unknown)")) << "\n";
}

void onWhois(const nlohmann::json &v)
{
    std::cout << "event:       onWhois\n";
    std::cout << "server:      " << json_util::pretty(v.value("server", "(unknown)")) << "\n";
    std::cout << "nickname:    " << json_util::pretty(v.value("nickname", "(unknown)")) << "\n";
    std::cout << "username:    " << json_util::pretty(v.value("username", "(unknown)")) << "\n";
    std::cout << "host:        " << json_util::pretty(v.value("host", "(unknown)")) << "\n";
    std::cout << "realname:    " << json_util::pretty(v.value("realname", "(unknown)")) << "\n";
}

const std::unordered_map<std::string, std::function<void (const nlohmann::json&)>> events{
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
    { "onTopic",            onTopic         },
    { "onWhois",            onWhois         }
};

void get_event(ctl::controller& ctl, std::string fmt)
{
    ctl.recv([&ctl, fmt] (auto code, auto message) {
        if (code)
            throw boost::system::system_error(code);

        const auto event = json_util::parser(message).get<std::string>("event");
        const auto it = events.find(event ? *event : "");

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

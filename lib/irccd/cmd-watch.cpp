/*
 * cmd-watch.cpp -- implementation of irccdctl watch
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

#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "cmd-watch.hpp"
#include "irccdctl.hpp"

namespace irccd {

namespace command {

namespace {

std::string dump(const nlohmann::json &object, const std::string &property)
{
    auto it = object.find(property);

    if (it == object.end())
        return "";

    return it->dump();
}

void onChannelMode(const nlohmann::json &v)
{
    std::cout << "event:       onChannelMode\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "mode:        " << dump(v, "mode") << "\n";
    std::cout << "argument:    " << dump(v, "argument") << "\n";
}

void onChannelNotice(const nlohmann::json &v)
{
    std::cout << "event:       onChannelNotice\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "channel:     " << dump(v, "channel") << "\n";
    std::cout << "message:     " << dump(v, "message") << "\n";
}

void onConnect(const nlohmann::json &v)
{
    std::cout << "event:       onConnect\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
}

void onInvite(const nlohmann::json &v)
{
    std::cout << "event:       onInvite\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "channel:     " << dump(v, "channel") << "\n";
}

void onJoin(const nlohmann::json &v)
{
    std::cout << "event:       onJoin\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "channel:     " << dump(v, "channel") << "\n";
}

void onKick(const nlohmann::json &v)
{
    std::cout << "event:       onKick\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "channel:     " << dump(v, "channel") << "\n";
    std::cout << "target:      " << dump(v, "target") << "\n";
    std::cout << "reason:      " << dump(v, "reason") << "\n";
}

void onMessage(const nlohmann::json &v)
{
    std::cout << "event:       onMessage\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "channel:     " << dump(v, "channel") << "\n";
    std::cout << "message:     " << dump(v, "message") << "\n";
}

void onMe(const nlohmann::json &v)
{
    std::cout << "event:       onMe\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "target:      " << dump(v, "target") << "\n";
    std::cout << "message:     " << dump(v, "message") << "\n";
}

void onMode(const nlohmann::json &v)
{
    std::cout << "event:       onMode\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "mode:        " << dump(v, "mode") << "\n";
}

void onNames(const nlohmann::json &v)
{
    std::cout << "event:       onNames\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "channel:     " << dump(v, "channel") << "\n";
    std::cout << "names:       " << dump(v, "names") << "\n";
}

void onNick(const nlohmann::json &v)
{
    std::cout << "event:       onNick\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "nickname:    " << dump(v, "nickname") << "\n";
}

void onNotice(const nlohmann::json &v)
{
    std::cout << "event:       onNotice\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "message:     " << dump(v, "message") << "\n";
}

void onPart(const nlohmann::json &v)
{
    std::cout << "event:       onPart\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "channel:     " << dump(v, "channel") << "\n";
    std::cout << "reason:      " << dump(v, "reason") << "\n";
}

void onQuery(const nlohmann::json &v)
{
    std::cout << "event:       onQuery\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "message:     " << dump(v, "message") << "\n";
}

void onTopic(const nlohmann::json &v)
{
    std::cout << "event:       onTopic\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "origin:      " << dump(v, "origin") << "\n";
    std::cout << "channel:     " << dump(v, "channel") << "\n";
    std::cout << "topic:       " << dump(v, "topic") << "\n";
}

void onWhois(const nlohmann::json &v)
{
    std::cout << "event:       onWhois\n";
    std::cout << "server:      " << dump(v, "server") << "\n";
    std::cout << "nickname:    " << dump(v, "nickname") << "\n";
    std::cout << "username:    " << dump(v, "username") << "\n";
    std::cout << "host:        " << dump(v, "host") << "\n";
    std::cout << "realname:    " << dump(v, "realname") << "\n";
}

const std::unordered_map<std::string, std::function<void (const nlohmann::json &)>> events{
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

} // !namespace

Watch::Watch()
    : Command(
        "watch", "General", "Start watching irccd events")
{
}

std::vector<Command::Option> Watch::options() const
{
    return {{ "format", "f", "format", "format", "output format" }};
}

nlohmann::json Watch::request(Irccdctl &ctl, const CommandRequest &request) const
{
    std::string format = request.optionOr("format", "native");

    if (format != "native" && format != "json")
        throw std::invalid_argument("invalid format given: " + format);

    while (ctl.connection().isConnected()) {
        try {
            auto object = ctl.next();
            auto event = object.find("event");

            if (event == object.end() || !event->is_string())
                continue;

            auto it = events.find(*event);

            // Silently ignore to avoid breaking user output.
            if (it == events.end())
                continue;

            if (format == "json")
                std::cout << object.dump() << std::endl;
            else {
                it->second(object);
                std::cout << std::endl;
            }
        } catch (...) {
        }
    }

    return nullptr;
}

} // !command

} // !irccd

/*
 * main.cpp -- irccd-test main file
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

#include <irccd/sysconfig.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#if defined(IRCCD_HAVE_LIBEDIT)
#   include <histedit.h>
#endif

#include <irccd/options.hpp>
#include <irccd/string_util.hpp>

#include <irccd/daemon/dynlib_plugin.hpp>
#include <irccd/daemon/irccd.hpp>

#include <irccd/daemon/service/plugin_service.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/debug_server.hpp>

#if defined(IRCCD_HAVE_JS)
#   include <irccd/js/js_plugin.hpp>
#endif

using boost::format;
using boost::str;

namespace irccd {

namespace su = string_util;

namespace {

boost::asio::io_service io;

std::unique_ptr<irccd> daemon;
std::shared_ptr<plugin> plugin;

// {{{ function table

/*
 * Forward declarations of handlers.
 */
void on_command(const std::string&);
void on_connect(const std::string&);
void on_invite(const std::string&);
void on_join(const std::string&);
void on_kick(const std::string&);
void on_load(const std::string&);
void on_me(const std::string&);
void on_message(const std::string&);
void on_mode(const std::string&);
void on_names(const std::string&);
void on_nick(const std::string&);
void on_notice(const std::string&);
void on_part(const std::string&);
void on_reload(const std::string&);
void on_topic(const std::string&);
void on_unload(const std::string&);
void on_whois(const std::string&);

/*
 * Table of user functions.
 */
using function = std::function<void (const std::string&)>;
using functions = std::unordered_map<std::string, function>;

static const functions list{
    { "onCommand",  &(on_command)   },
    { "onConnect",  &(on_connect)   },
    { "onInvite",   &(on_invite)    },
    { "onJoin",     &(on_join)      },
    { "onKick",     &(on_kick)      },
    { "onLoad",     &(on_load)      },
    { "onMe",       &(on_me)        },
    { "onMessage",  &(on_message)   },
    { "onMode",     &(on_mode)      },
    { "onNames",    &(on_names)     },
    { "onNick",     &(on_nick)      },
    { "onNotice",   &(on_notice)    },
    { "onPart",     &(on_part)      },
    { "onReload",   &(on_reload)    },
    { "onTopic",    &(on_topic)     },
    { "onUnload",   &(on_unload)    },
    { "onWhois",    &(on_whois)     }
};

// }}}

// {{{ usage

void usage()
{
    std::cerr << "usage: irccd-test [-c config] plugin-name" << std::endl;
    std::exit(1);
}

// }}}

// {{{ get_server

std::shared_ptr<server> get_server(std::string name)
{
    name = boost::algorithm::trim_copy(name);

    if (name.empty())
        name = "test";

    auto s = daemon->servers().get(name);

    if (!s) {
        s = std::make_shared<debug_server>(io, std::move(name), "localhost");
        daemon->servers().add(s);
    }

    return s;
}

// }}}

// {{{ get_arg

std::string get_arg(const std::vector<std::string>& args, unsigned index)
{
    if (index >= args.size())
        return "";

    return args[index];
}

// }}}

// {{{ on_command

/*
 * onCommand server origin channel message
 */
void on_command(const std::string& data)
{
    const auto args = su::split(data, " ", 4);

    plugin->handle_command(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3)
    });
}

// }}}

// {{{ on_connect

/*
 * onConnect server
 */
void on_connect(const std::string& data)
{
    const auto args = su::split(data, " ");

    plugin->handle_connect(*daemon, {get_server(get_arg(args, 0))});
}

// }}}

// {{{ on_invite

/*
 * onInvite server origin channel target
 */
void on_invite(const std::string& data)
{
    const auto args = su::split(data, " ");

    plugin->handle_invite(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3),
    });
}

// }}}

// {{{ on_join

/*
 * onJoin server origin channel
 */
void on_join(const std::string& data)
{
    const auto args = su::split(data, " ");

    plugin->handle_join(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2)
    });
}

// }}}

// {{{ on_kick

/*
 * onKick server origin channel reason
 */
void on_kick(const std::string& data)
{
    const auto args = su::split(data, " ", 5);

    plugin->handle_kick(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3),
        get_arg(args, 4),
    });
}

// }}}

// {{{ on_load

/*
 * onLoad
 */
void on_load(const std::string&)
{
    plugin->handle_load(*daemon);
}

// }}}

// {{{ on_me

/*
 * onMe server origin channel message
 */
void on_me(const std::string& data)
{
    const auto args = su::split(data, " ", 4);

    plugin->handle_me(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3)
    });
}

// }}}

// {{{ on_message

/*
 * onMessage server origin channel message
 */
void on_message(const std::string& data)
{
    const auto args = su::split(data, " ", 4);

    plugin->handle_message(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3)
    });
}

// }}}

// {{{ on_mode

/*
 * onMode server origin channel mode limit user mask
 */
void on_mode(const std::string& data)
{
    const auto args = su::split(data, " ", 7);

    plugin->handle_mode(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3),
        get_arg(args, 4),
        get_arg(args, 5),
        get_arg(args, 6),
    });
}

// }}}

// {{{ on_names

/*
 * onNames server channel nick1 nick2 nickN
 */
void on_names(const std::string& data)
{
    const auto args = su::split(data, " ");

    names_event ev;

    ev.server = get_server(get_arg(args, 0));
    ev.channel = get_arg(args, 1);

    if (args.size() >= 3U)
        ev.names.insert(ev.names.begin(), args.begin() + 2, args.end());

    plugin->handle_names(*daemon, ev);
}

// }}}

// {{{ on_nick

/*
 * onNick server origin nickname
 */
void on_nick(const std::string& data)
{
    const auto args = su::split(data, " ");

    plugin->handle_nick(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2)
    });
}

// }}}

// {{{ on_notice

/*
 * onNotice server origin channel nickname
 */
void on_notice(const std::string& data)
{
    const auto args = su::split(data, " ", 4);

    plugin->handle_notice(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3)
    });
}

// }}}

// {{{ on_part

/*
 * onPart server origin channel reason
 */
void on_part(const std::string& data)
{
    const auto args = su::split(data, " ", 4);

    plugin->handle_part(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3),
    });
}

// }}}

// {{{ on_reload

/*
 * onReload
 */
void on_reload(const std::string&)
{
    plugin->handle_reload(*daemon);
}

// }}}

// {{{ on_topic

/*
 * onTopic server origin channel topic
 */
void on_topic(const std::string& data)
{
    const auto args = su::split(data, " ", 4);

    plugin->handle_topic(*daemon, {
        get_server(get_arg(args, 0)),
        get_arg(args, 1),
        get_arg(args, 2),
        get_arg(args, 3)
    });
}

// }}}

// {{{ on_unload

/*
 * onUnload
 */
void on_unload(const std::string&)
{
    plugin->handle_unload(*daemon);
}

// }}}

// {{{ on_whois

/*
 * onWhois server nick user host realname chan1 chan2 chanN
 */
void on_whois(const std::string& data)
{
    const auto args = su::split(data, " ");

    whois_event ev;

    ev.server = get_server(get_arg(args, 0));
    ev.whois.nick = get_arg(args, 1);
    ev.whois.user = get_arg(args, 2);
    ev.whois.host = get_arg(args, 3);
    ev.whois.realname = get_arg(args, 4);

    if (args.size() >= 5)
        ev.whois.channels.insert(ev.whois.channels.begin(), args.begin() + 5, args.end());

    plugin->handle_whois(*daemon, ev);
}

// }}}

// {{{ exec

void exec(const std::string& line)
{
    const auto pos = line.find(' ');
    const auto it = list.find(line.substr(0, pos));

    if (it != list.end())
        it->second(pos == std::string::npos ? "" : line.substr(pos + 1));
}

// }}}

#if defined(IRCCD_HAVE_LIBEDIT)

// {{{ prompt (libedit version)

const char* prompt(EditLine*)
{
    static const char* text = "> ";

    return text;
}

std::string clean(std::string input)
{
    while (!input.empty() && (input.back() == '\n' || input.back() == '\r'))
        input.pop_back();

    return input;
}

std::vector<std::string> matches(const std::string& name)
{
    std::vector<std::string> result;

    for (const auto& pair : list)
        if (pair.first.compare(0U, name.size(), name) == 0U)
            result.push_back(pair.first);

    return result;
}

unsigned char complete(EditLine* el, int)
{
    const auto* lf = el_line(el);
    const auto args = su::split(std::string(lf->buffer, lf->cursor), " ");

    if (args.size() == 0U)
        return CC_REFRESH;

    const auto found = matches(args[0]);

    if (found.size() != 1U)
        return CC_REFRESH;

    // Insert the missing text, e.g. onCom -> onCommand.
    if (el_insertstr(el, &found[0].c_str()[args[0].size()]) < 0)
        return CC_ERROR;

    return CC_REFRESH;
}

void run()
{
    std::unique_ptr<EditLine, void (*)(EditLine*)> el(
        el_init("irccd-test", stdin, stdout, stderr),
        el_end
    );
    std::unique_ptr<History, void (*)(History*)> hist(
        history_init(),
        history_end
    );
    HistEvent hev;

    history(hist.get(), &hev, H_SETSIZE, 1024);
    el_set(el.get(), EL_EDITOR, "emacs");
    el_set(el.get(), EL_PROMPT, prompt);
    el_set(el.get(), EL_HIST, history, hist.get());
    el_set(el.get(), EL_ADDFN, "ed-complete", "Complete command", complete);
    el_set(el.get(), EL_BIND, "^I", "ed-complete", nullptr);

    const char* s;
    int size;

    while ((s = el_gets(el.get(), &size)) && size >= 0) {
        if (size > 0)
            history(hist.get(), &hev, H_ENTER, s);

        exec(clean(s));
    }
}

// }}}

#else

// {{{ run (standard version)

void run()
{
    std::string line;

    for (;;) {
        std::cout << "> ";

        if (!std::getline(std::cin, line))
            return;

        exec(line);
    }
}

// }}}

#endif

// {{{ load_plugins

void load_plugins(int argc, char** argv)
{
    if (argc <= 0)
        usage();

    daemon->plugins().load("test", boost::filesystem::exists(argv[0]) ? argv[0] : "");
    plugin = daemon->plugins().get("test");
}

// }}}

// {{{ load_config

auto load_config(const option::result& result) -> config
{
    auto it = result.find("-c");

    if (it != result.end() || (it = result.find("--config")) != result.end())
        return config(it->second);

    auto cfg = config::search("irccd.conf");

    return *cfg;
}

// }}}

// {{{ load_options

void load_options(int& argc, char**& argv)
{
    static const option::options def{
        { "-c",         true    },
        { "--config",   true    }
    };

    try {
        daemon->set_config(load_config(option::read(argc, argv, def)));
    } catch (const std::exception& ex) {
        throw std::runtime_error(str(format("%1%") % ex.what()));
    }
}

// }}}

// {{{ load

void load(int argc, char** argv)
{
    daemon = std::make_unique<irccd>(io);

#if defined(IRCCD_HAVE_JS)
    daemon->plugins().add_loader(js_plugin_loader::defaults(*daemon));
#endif

    load_options(argc, argv);
    load_plugins(argc, argv);
}

// }}}

} // !namespace

} // !irccd

int main(int argc, char** argv)
{
    try {
        irccd::load(--argc, ++argv);
        irccd::run();
    } catch (const std::exception& ex) {
        std::cerr << "abort: " << ex.what() << std::endl;
        return 1;
    }
}

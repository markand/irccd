/*
 * sample_hook.cpp -- sample hook for unit tests
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#include <iostream>
#include <functional>
#include <string_view>
#include <unordered_map>

using std::function;
using std::string_view;
using std::unordered_map;
using std::cerr;
using std::cout;
using std::endl;

namespace {

auto print(int argc, char** argv, int index) -> string_view
{
	return argc > index ? argv[index] : "";
}

void handle_onConnect(int argc, char** argv)
{
	cout << "event:   onConnect" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
}

void handle_onDisconnect(int argc, char** argv)
{
	cout << "event:   onDisconnect" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
}

void handle_onInvite(int argc, char** argv)
{
	cout << "event:   onInvite" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
	cout << "target:  " << print(argc, argv, 3) << endl;
}

void handle_onJoin(int argc, char** argv)
{
	cout << "event:   onJoin" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
}

void handle_onKick(int argc, char** argv)
{
	cout << "event:   onKick" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
	cout << "target:  " << print(argc, argv, 3) << endl;
	cout << "reason:  " << print(argc, argv, 4) << endl;
}

void handle_onMessage(int argc, char** argv)
{
	cout << "event:   onMessage" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
	cout << "message: " << print(argc, argv, 3) << endl;
}

void handle_onMe(int argc, char** argv)
{
	cout << "event:   onMe" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
	cout << "message: " << print(argc, argv, 3) << endl;
}

void handle_onMode(int argc, char** argv)
{
	cout << "event:   onMode" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
	cout << "mode:    " << print(argc, argv, 3) << endl;
	cout << "limit:   " << print(argc, argv, 4) << endl;
	cout << "user:    " << print(argc, argv, 5) << endl;
	cout << "mask:    " << print(argc, argv, 6) << endl;
}

void handle_onNick(int argc, char** argv)
{
	cout << "event:   onNick" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "nick:    " << print(argc, argv, 2) << endl;
}

void handle_onNotice(int argc, char** argv)
{
	cout << "event:   onNotice" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
	cout << "message: " << print(argc, argv, 3) << endl;
}

void handle_onPart(int argc, char** argv)
{
	cout << "event:   onPart" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
	cout << "reason:  " << print(argc, argv, 3) << endl;
}

void handle_onTopic(int argc, char** argv)
{
	cout << "event:   onTopic" << endl;
	cout << "server:  " << print(argc, argv, 0) << endl;
	cout << "origin:  " << print(argc, argv, 1) << endl;
	cout << "channel: " << print(argc, argv, 2) << endl;
	cout << "topic:   " << print(argc, argv, 3) << endl;
}

unordered_map<string_view, function<void (int, char**)>> handlers{
	{ "onConnect",    handle_onConnect          },
	{ "onDisconnect", handle_onDisconnect       },
	{ "onInvite",     handle_onInvite           },
	{ "onJoin",       handle_onJoin             },
	{ "onKick",       handle_onKick             },
	{ "onMessage",    handle_onMessage          },
	{ "onMe",         handle_onMe               },
	{ "onMode",       handle_onMode             },
	{ "onNick",       handle_onNick             },
	{ "onNotice",     handle_onNotice           },
	{ "onPart",       handle_onPart             },
	{ "onTopic",      handle_onTopic            }
};

} // !namespace

int main(int argc, char** argv)
{
	--argc;
	++argv;

	if (argc == 0) {
		cerr << "abort: no command given" << endl;
		return 1;
	}

	const auto handler = handlers.find(argv[0]);

	if (handler == handlers.end()) {
		cerr << "abort: unknown message hook: " << argv[0] << endl;
		return 1;
	}

	handler->second(--argc, ++argv);
}

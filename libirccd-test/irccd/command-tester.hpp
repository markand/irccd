/*
 * command-tester.hpp -- test fixture helper for remote commands
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

#ifndef IRCCD_COMMAND_TESTER_HPP
#define IRCCD_COMMAND_TESTER_HPP

#include <gtest/gtest.h>

#include "elapsed-timer.hpp"
#include "irccd.hpp"
#include "irccdctl.hpp"
#include "util.hpp"

namespace irccd {

class Command;
class Server;

class CommandTester : public testing::Test {
protected:
    Irccd m_irccd;
    Irccdctl m_irccdctl;


public:
    CommandTester(std::unique_ptr<Command> cmd = nullptr,
                  std::unique_ptr<Server> server = nullptr);


    template <typename Predicate>
    void poll(Predicate &&predicate)
    {
        ElapsedTimer timer;

        while (!predicate() && timer.elapsed() < 30000)
            util::poller::poll(250, m_irccd, m_irccdctl);
    }
};

} // !irccd

#endif // !IRCCD_COMMAND_TESTER_HPP

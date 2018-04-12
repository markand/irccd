/*
 * rule_cli_test.hpp -- test fixture for irccdctl frontend (rule support)
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

#ifndef IRCCD_TEST_RULE_CLI_TEST_HPP
#define IRCCD_TEST_RULE_CLI_TEST_HPP

/**
 * \file rule_cli_test.hpp
 * \brief Test fixture for irccdctl frontend (rule support).
 */

#include <irccd/daemon/service/rule_service.hpp>

#include <irccd/daemon/rule.hpp>

#include "cli_test.hpp"

namespace irccd {

/**
 * \file rule_cli_test.hpp
 *
 * This class adds all rule related transport commands to irccd.
 */
class rule_cli_test : public cli_test {
public:
    /**
     * Default constructor.
     */
    rule_cli_test();
};

} // !irccd

#endif // !IRCCD_TEST_RULE_CLI_TEST_HPP

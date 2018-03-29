/*
 * cli_test.hpp -- test fixture for irccdctl frontend
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

#ifndef IRCCD_TEST_CLI_TEST_HPP
#define IRCCD_TEST_CLI_TEST_HPP

/**
 * \file cli_test.hpp
 * \brief Test fixture for irccdctl frontend.
 */

#include <utility>
#include <vector>

#include <boost/process.hpp>

namespace irccd {

/**
 * \brief Test fixture for irccdctl frontend.
 */
class cli_test {
private:
    boost::process::child irccd_;

public:
    /**
     * Type for all lines printed.
     */
    using output = std::vector<std::string>;

    /**
     * Collection of output from stdout/stderr respectively.
     */
    using outputs = std::pair<output, output>;

    /**
     * Start irccd daemon with the configuration file (relative to tests/data).
     *
     * \param config the path to the config
     */
    void run_irccd(const std::string& config);

    /**
     * Start irccdctl and returns its stdout/stderr.
     *
     * \param args the arguments to irccdctl
     */
    outputs run_irccdctl(const std::vector<std::string>& args);

    /**
     * Convenient function that starts irccd and irccdctl for oneshot tests.
     *
     * \param config the base configuration name for irccd
     * \param args the arguments to irccdctl
     */
    outputs run(const std::string& config, const std::vector<std::string>& args);
};

} // !irccd

#endif // !IRCCD_TEST_CLI_TEST_HPP

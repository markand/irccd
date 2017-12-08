/*
 * plugin_load_cli.hpp -- implementation of irccdctl plugin-load
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

#ifndef IRCCD_CTL_PLUGIN_LOAD_CLI_HPP
#define IRCCD_CTL_PLUGIN_LOAD_CLI_HPP

/**
 * \file plugin_load_cli.hpp
 * \brief Implementation of irccdctl plugin-load.
 */

#include "cli.hpp"

namespace irccd {

namespace ctl {

/**
 * \brief Implementation of irccdctl plugin-load.
 */
class plugin_load_cli : public cli {
public:
    /**
     * \copydoc cli::name
     */
    std::string name() const override;

    /**
     * \copydoc cli::exec
     */
    void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_PLUGIN_LOAD_CLI_HPP

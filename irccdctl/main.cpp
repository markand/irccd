/*
 * main.cpp -- irccd controller main
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

#include <irccd/irccdctl.hpp>
#include <irccd/logger.hpp>
#include <irccd/path.hpp>
#include <irccd/system.hpp>

using namespace irccd;

int main(int argc, char **argv)
{
    // TODO: move to Application
    sys::setProgramName("irccdctl");
    path::setApplicationPath(argv[0]);
    log::setVerbose(false);

    try {
        Irccdctl ctl;

        ctl.run(--argc, ++argv);
    } catch (const std::exception &ex) {
        log::warning() << "error: " << ex.what() << std::endl;
        std::exit(1);
    }

    return 0;
}

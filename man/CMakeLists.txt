#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

project(man)

set(
	MANPAGES
	irccd-api-chrono.3
	irccd-api-directory.3
	irccd-api-file.3
	irccd-api-logger.3
	irccd-api-plugin.3
	irccd-api-server.3
	irccd-api-system.3
	irccd-api-timer.3
	irccd-api-unicode.3
	irccd-api-util.3
	irccd-api.3
	irccd-ipc.7
	irccd-templates.7
	irccd-test.1
	irccd.1
	irccd.conf.5
	irccdctl.1
	irccdctl.conf.5
)

foreach (m ${MANPAGES})
	string(REGEX REPLACE ".*\\.([0-9])$" "\\1" section ${m})
	irccd_define_man(INPUT ${m} SECTION man${section})
endforeach ()

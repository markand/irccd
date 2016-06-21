#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#
# Install manuals
# -------------------------------------------------------------------

set(
    MONTHS
    January
    February
    March
    April
    May
    June
    July
    August
    September
    October
    November
    December
)

# Get month as name.
math(EXPR INDEX "${IRCCD_RELEASE_DATE_MONTH} - 1")
list(GET MONTHS ${INDEX} MONTH)

set(YEAR ${IRCCD_RELEASE_DATE_YEAR})
set(DAY ${IRCCD_RELEASE_DATE_DAY})

# Update man pages date
irccd_define_man(irccd.1 man1)
irccd_define_man(irccd.conf.5 man5)
irccd_define_man(irccdctl.1 man1)
irccd_define_man(irccdctl.conf.5 man5)

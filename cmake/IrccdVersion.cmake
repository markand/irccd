#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

# Irccd version.
set(IRCCD_VERSION_MAJOR "2")
set(IRCCD_VERSION_MINOR "1")
set(IRCCD_VERSION_PATCH "1")
set(IRCCD_VERSION "${IRCCD_VERSION_MAJOR}.${IRCCD_VERSION_MINOR}.${IRCCD_VERSION_PATCH}")
set(IRCCD_VERSION_SHLIB "2")

#
# Irccd release date.
# -------------------------------------------------------------------
#
# IRCCD_RELEASE_DATE_YEAR       4 digits
# IRCCD_RELEASE_DATE_MONTH      2 digits (01 = January)
# IRCCD_RELEASE_DATE_DAY        2 digits (01 = first day of month)
#
set(IRCCD_RELEASE_DATE_YEAR 2017)
set(IRCCD_RELEASE_DATE_MONTH 03)
set(IRCCD_RELEASE_DATE_DAY 07)
set(IRCCD_RELEASE_DATE "${IRCCD_RELEASE_DATE_YEAR}-${IRCCD_RELEASE_DATE_MONTH}-${IRCCD_RELEASE_DATE_DAY}")

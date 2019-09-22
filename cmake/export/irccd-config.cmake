#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

include(CMakeFindDependencyMacro)

find_dependency(Boost COMPONENTS date_time filesystem system timer)
find_dependency(Threads)
find_dependency(OpenSSL)

set(IRCCD_WITH_JS @IRCCD_WITH_JS@)

include("${CMAKE_CURRENT_LIST_DIR}/irccd-targets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/IrccdSetGlobal.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/IrccdDefinePlugin.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/IrccdDefineMan.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/IrccdDefineExecutable.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/IrccdDefineLibrary.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/IrccdDefineMan.cmake")

#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

project(irccdctl)

add_executable(irccdctl ${irccdctl_SOURCE_DIR}/irccdctl.c)
target_link_libraries(irccdctl libirccd)
source_group(TREE ${irccdctl_SOURCE_DIR} FILES ${irccdctl_SOURCE_DIR}/irccdctl.c)
install(TARGETS irccdctl DESTINATION ${CMAKE_INSTALL_BINDIR})

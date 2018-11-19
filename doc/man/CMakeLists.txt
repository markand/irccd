#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

# irccd(1)
irccd_define_man(
    ${doc_SOURCE_DIR}/src/irccd.md
    ${man_BINARY_DIR}/irccd.1
    ${man_SOURCE_DIR}/template-irccd.1
    "man1"
)

# irccdctl(1)
irccd_define_man(
    ${doc_SOURCE_DIR}/src/irccdctl.md
    ${man_BINARY_DIR}/irccdctl.1
    ${man_SOURCE_DIR}/template-irccdctl.1
    "man1"
)

# irccd.templates(7)
irccd_define_man(
    ${doc_SOURCE_DIR}/src/templates.md
    ${man_BINARY_DIR}/irccd.templates.7
    ${man_SOURCE_DIR}/template-irccd.templates.7
    "man7"
)

# irccd.ipc(3)
irccd_define_man(
    ${doc_SOURCE_DIR}/src/ipc.md
    ${man_BINARY_DIR}/irccd.ipc.3
    ${man_SOURCE_DIR}/template-irccd.ipc.3
    "man3"
)

add_custom_target(
    docs-man
    SOURCES
        ${man_BINARY_DIR}/irccd.1
        ${man_BINARY_DIR}/irccd.ipc.3
        ${man_BINARY_DIR}/irccd.templates.7
        ${man_BINARY_DIR}/irccdctl.1
        ${man_SOURCE_DIR}/template-irccd.1
        ${man_SOURCE_DIR}/template-irccd.ipc.3
        ${man_SOURCE_DIR}/template-irccd.templates.7
        ${man_SOURCE_DIR}/template-irccdctl.1
)

add_dependencies(docs docs-man)

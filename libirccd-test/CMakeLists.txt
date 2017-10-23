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

project(libirccd-test)

irccd_define_library(
    TARGET libirccd-test
    SOURCES
        ${libirccd-test_SOURCE_DIR}/irccd/command-tester.cpp
        ${libirccd-test_SOURCE_DIR}/irccd/command-tester.hpp
        ${libirccd-test_SOURCE_DIR}/irccd/journal_server.cpp
        ${libirccd-test_SOURCE_DIR}/irccd/journal_server.hpp
        $<$<BOOL:${HAVE_JS}>:${libirccd-test_SOURCE_DIR}/irccd/plugin-tester.cpp>
        $<$<BOOL:${HAVE_JS}>:${libirccd-test_SOURCE_DIR}/irccd/plugin-tester.hpp>
        $<$<BOOL:${HAVE_JS}>:${libirccd-test_SOURCE_DIR}/irccd/plugin_test.cpp>
        $<$<BOOL:${HAVE_JS}>:${libirccd-test_SOURCE_DIR}/irccd/plugin_test.hpp>
        $<$<BOOL:${HAVE_JS}>:${libirccd-test_SOURCE_DIR}/irccd/js_test.hpp>
        $<$<BOOL:${HAVE_JS}>:${libirccd-test_SOURCE_DIR}/irccd/plugin_test.js>
        ${libirccd-test_SOURCE_DIR}/irccd/server-tester.cpp
        ${libirccd-test_SOURCE_DIR}/irccd/server-tester.hpp
    LIBRARIES
        $<$<BOOL:${HAVE_JS}>:libirccd-js>
        libirccd
        libirccdctl
        extern-gtest
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${libirccd-test_SOURCE_DIR}/irccd>
    FLAGS
        $<BUILD_INTERFACE:IRCCD_PLUGIN_TEST="${libirccd-test_SOURCE_DIR}/irccd/plugin_test.js">
)

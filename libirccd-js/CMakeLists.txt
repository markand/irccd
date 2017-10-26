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

project(libirccd-js)

find_package(Boost REQUIRED QUIET COMPONENTS timer)

set(
    HEADERS
    ${libirccd-js_SOURCE_DIR}/irccd/duktape.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_directory_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_elapsed_timer_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_file_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_irccd_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_logger_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_plugin_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_server_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_system_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_timer_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_unicode_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_util_module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_plugin.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/timer.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/unicode.hpp
)

set(
    SOURCES
    ${libirccd-js_SOURCE_DIR}/irccd/js_directory_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_elapsed_timer_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_file_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_irccd_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_logger_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_plugin_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_server_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_system_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_timer_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_unicode_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_util_module.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/js_plugin.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/timer.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/unicode.cpp
)

irccd_define_library(
    TARGET libirccd-js
    SOURCES
        ${libirccd-js_SOURCE_DIR}/CMakeLists.txt
        ${HEADERS}
        ${SOURCES}
    LIBRARIES
        Boost::timer
        extern-duktape
        libirccd
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${libirccd-js_SOURCE_DIR}/irccd>
        $<BUILD_INTERFACE:${libirccd-js_SOURCE_DIR}>
)

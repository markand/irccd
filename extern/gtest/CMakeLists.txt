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

project(gtest)

irccd_define_library(
    LOCAL
    TARGET extern-gtest
    SOURCES src/gtest-all.cc
    PUBLIC_INCLUDES ${gtest_SOURCE_DIR}/include
    LOCAL_INCLUDES ${gtest_SOURCE_DIR}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(extern-gtest PRIVATE -Wno-missing-field-initializers)
endif ()

set_target_properties(
    extern-gtest
    PROPERTIES
        PROJECT_LABEL gtest
        FOLDER extern
)
#
# IrccdDefineLibrary.cmake -- CMake build system for irccd
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
# irccd_define_library
# -------------------------------------------------------------------
#
# irccd_define_library(
#    TARGET target name
#    SOURCES src1, src2, srcn
#    LOCAL (Optional) set to true to build a static library
#    FLAGS (Optional) C/C++ flags (without -D)
#    LIBRARIES (Optional) libraries to link
#    LOCAL_INCLUDES (Optional) local includes for the target only
#    PUBLIC_INCLUDES (Optional) includes to share with target dependencies
# )
#
# Create a static library for internal use.
#

function(irccd_define_library)
    set(options LOCAL)
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES FLAGS LIBRARIES LOCAL_INCLUDES PUBLIC_INCLUDES)
    set(mandatory TARGET SOURCES)

    cmake_parse_arguments(LIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT LIB_TARGET)
        message(FATAL_ERROR "Please set TARGET")
    endif ()
    if (NOT LIB_SOURCES)
        message(FATAL_ERROR "Please set SOURCES")
    endif ()
    if (LIB_LOCAL)
        set(type STATIC)
    endif ()

    add_library(${LIB_TARGET} ${type} ${LIB_SOURCES})
    target_include_directories(${LIB_TARGET} PRIVATE ${LIB_LOCAL_INCLUDES} PUBLIC ${LIB_PUBLIC_INCLUDES})
    target_compile_definitions(${LIB_TARGET} PUBLIC ${LIB_FLAGS})
    target_link_libraries(${LIB_TARGET} ${LIB_LIBRARIES})
    set_target_properties(${LIB_TARGET} PROPERTIES PREFIX "")
endfunction()

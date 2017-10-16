#
# IrccdDefineExecutable.cmake -- CMake build system for irccd
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

#
# irccd_define_executable
# -------------------------------------------------------------------
#
# irccd_define_executable(
#    TARGET target name
#    DESCRIPTION short description (Required if installed)
#    SOURCES src1, src2, srcn
#    FLAGS (Optional) C/C++ flags (without -D)
#    LIBRARIES (Optional) libraries to link
#    INCLUDES (Optional) includes for the target
# )
#
# Create an executable that can be installed or not.
#

function(irccd_define_executable)
    set(options "")
    set(oneValueArgs DESCRIPTION TARGET)
    set(multiValueArgs SOURCES FLAGS LIBRARIES INCLUDES)

    cmake_parse_arguments(EXE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT EXE_TARGET)
        message(FATAL_ERROR "Please set TARGET")
    endif ()
    if (NOT EXE_SOURCES)
        message(FATAL_ERROR "Please set SOURCES")
    endif ()
    if (NOT EXE_DESCRIPTION)
        message(FATAL_ERROR "DESCRIPTION required")
    endif ()

    add_executable(${EXE_TARGET} ${EXE_SOURCES})
    target_include_directories(${EXE_TARGET} PRIVATE ${EXE_INCLUDES})
    target_compile_definitions(${EXE_TARGET} PRIVATE ${EXE_FLAGS})
    target_link_libraries(${EXE_TARGET} ${EXE_LIBRARIES})

    set_target_properties(
        ${EXE_TARGET}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    foreach (c ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${c} cu)
        set_target_properties(
            ${EXE_TARGET}
            PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${cu} ${CMAKE_BINARY_DIR}/bin/${c}
        )
    endforeach()

    install(
        TARGETS ${EXE_TARGET}
        COMPONENT ${EXE_TARGET}
        RUNTIME DESTINATION ${WITH_BINDIR}
    )

    # Put the application into a cpack group.
    string(TOUPPER ${EXE_TARGET} CMP)
    setg(CPACK_COMPONENT_${CMP}_DISPLAY_NAME "${EXE_TARGET} executable")
    setg(CPACK_COMPONENT_${CMP}_DESCRIPTION ${EXE_DESCRIPTION})
    setg(CPACK_COMPONENT_${CMP}_GROUP "Applications")
endfunction()

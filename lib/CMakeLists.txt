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

project(lib)

include(irccd/CMakeSources.cmake)

add_library(libirccd ${HEADERS} ${SOURCES})

target_include_directories(
    libirccd
    PUBLIC
        $<BUILD_INTERFACE:${duktape_SOURCE_DIR}>
        $<BUILD_INTERFACE:${extern-libircclient_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${IRCCD_FAKEROOTDIR}/include/irccd>
        $<BUILD_INTERFACE:${IRCCD_FAKEROOTDIR}/include>
        $<BUILD_INTERFACE:${lib_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

source_group(irccd FILES ${HEADERS} ${SOURCES})

if (IRCCD_SYSTEM_WINDOWS)
    list(APPEND LIBRARIES ws2_32 shlwapi)

    if (BUILD_SHARED_LIBS)
        list(APPEND FLAGS IRCCD_BUILDING_DLL)
    endif ()
elseif (IRCCD_SYSTEM_MAC)
    list(APPEND LIBRARIES resolv)
elseif (IRCCD_SYSTEM_LINUX)
    list(APPEND LIBRARIES dl)
endif ()

if (OPENSSL_FOUND)
    list(APPEND LIBRARIES OpenSSL::SSL OpenSSL::Crypto)
else ()
    list(APPEND FLAGS NET_NO_SSL)
endif ()

target_link_libraries(libirccd extern-duktape extern-ircclient extern-json extern-fmt ${LIBRARIES})
target_compile_definitions(libirccd PRIVATE ${FLAGS})

set_target_properties(
    libirccd
    PROPERTIES
        PREFIX ""
        OUTPUT_NAME_DEBUG libirccd2d
        RUNTIME_OUTPUT_DIRECTORY ${IRCCD_FAKEROOTDIR}/${WITH_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${IRCCD_FAKEROOTDIR}/${WITH_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${IRCCD_FAKEROOTDIR}/${WITH_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${IRCCD_FAKEROOTDIR}/${WITH_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${IRCCD_FAKEROOTDIR}/${WITH_BINDIR}
        VERSION ${IRCCD_VERSION}
        SOVERSION ${IRCCD_VERSION_SHLIB}
)

install(
    TARGETS libirccd
    COMPONENT libirccd
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

setg(CPACK_COMPONENT_LIBIRCCD_GROUP "Applications")
setg(CPACK_COMPONENT_LIBIRCCD_DISPLAY_NAME "Irccd library")
setg(CPACK_COMPONENT_LIBIRCCD_DESCRIPTION "Irccd runtime library")
setg(CPACK_COMPONENT_LIBIRCCD_REQUIRED On)
setg(CPACK_COMPONENT_LIBIRCCD_HIDDEN On)

#
# Headers.
# -------------------------------------------------------------------
#

install(
    FILES ${HEADERS}
    COMPONENT headers
    DESTINATION include/irccd
)

setg(CPACK_COMPONENT_HEADERS_GROUP "Development")
setg(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ headers")
setg(CPACK_COMPONENT_HEADERS_DESCRIPTION
    "Install the C++ headers if you want to build irccd native plugins.")

# # CMake target exports.
# export(
#     EXPORT Irccd2Targets
#     FILE "${CMAKE_BINARY_DIR}/Irccd2Targets.cmake"
#     NAMESPACE Irccd2::
# )

# CMake version file.
# include(CMakePackageConfigHelpers)

# write_basic_package_version_file(
#     "${CMAKE_CURRENT_BINARY_DIR}/Irccd2ConfigVersion.cmake"
#     VERSION ${IRCCD_VERSION}
#     COMPATIBILITY AnyNewerVersion
# )

# Irccd main module file.
# configure_file(
#     "${CMAKE_SOURCE_DIR}/cmake/internal/Irccd2Config.cmake.in"
#     "${CMAKE_BINARY_DIR}/Irccd2Config.cmake"
#     COPYONLY
# )

# Install all CMake stuff.
# install(
#     EXPORT Irccd2Targets
#     FILE Irccd2Targets.cmake
#     NAMESPACE Irccd2::
#     DESTINATION ${WITH_CMAKEDIR}/Irccd2
# )
#
# install(
#     FILES
#         "${CMAKE_BINARY_DIR}/Irccd2Config.cmake"
#         "${CMAKE_BINARY_DIR}/Irccd2ConfigVersion.cmake"
#     DESTINATION ${WITH_CMAKEDIR}/Irccd2
# )

# pkg-config
if (WITH_PKGCONFIG)
    # Configure the pkg-config file.
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/internal/irccd2.pc.in
        ${CMAKE_BINARY_DIR}/irccd2.pc
        @ONLY
    )

    # Adjust the library output name if debug.
    file(
        GENERATE
        OUTPUT ${CMAKE_BINARY_DIR}/irccd2.pc
        INPUT ${CMAKE_BINARY_DIR}/irccd2.pc
    )

    install(
        FILES ${CMAKE_BINARY_DIR}/irccd2.pc
        COMPONENT pkgconfig
        DESTINATION ${WITH_PKGCONFIGDIR}
    )

    setg(CPACK_COMPONENT_PKGCONFIG_DESCRIPTION "Install .pc pkgconfig file")
    setg(CPACK_COMPONENT_PKGCONFIG_GROUP "Development")
endif ()

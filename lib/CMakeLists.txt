#
# CMakeLists.txt -- CMake build for irccd
#
# Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

project(libirccd)

include(CMakePackageConfigHelpers)

set(
	HEADERS
	irccd/channel.h
	irccd/conn.h
	irccd/event.h
	irccd/hook.h
	irccd/irccd.h
	irccd/limits.h
	irccd/log.h
	irccd/plugin.h
	irccd/rule.h
	irccd/server.h
	irccd/subst.h
	irccd/util.h
)

set(
	SOURCES
	CMakeLists.txt
	irccd/channel.c
	irccd/config.h.in
	irccd/conn.c
	irccd/event.c
	irccd/hook.c
	irccd/irccd.c
	irccd/log.c
	irccd/plugin.c
	irccd/rule.c
	irccd/server.c
	irccd/subst.c
	irccd/util.c
)

configure_file(
	${libirccd_SOURCE_DIR}/irccd/config.h.in
	${libirccd_BINARY_DIR}/irccd/config.h
)

add_library(libirccd-static ${SOURCES} ${HEADERS})
set_target_properties(libirccd-static PROPERTIES PREFIX "")

# This is what we export to the world.
add_library(libirccd INTERFACE)
add_library(irccd::libirccd ALIAS libirccd)

target_include_directories(libirccd INTERFACE
	$<BUILD_INTERFACE:${libirccd-compat_BINARY_DIR}>
	$<BUILD_INTERFACE:${libirccd_SOURCE_DIR}>
	$<BUILD_INTERFACE:${libirccd_BINARY_DIR}>
	$<BUILD_INTERFACE:${libirccd_BINARY_DIR}/irccd>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/irccd/extern>
	${OPENSSL_INCLUDE_DIR}
)

target_include_directories(
	libirccd-static
	PUBLIC
		$<BUILD_INTERFACE:${libirccd_BINARY_DIR}/irccd>
		$<BUILD_INTERFACE:${libirccd_SOURCE_DIR}>
		$<BUILD_INTERFACE:${libirccd_BINARY_DIR}>
)
target_link_libraries(libirccd-static libirccd-compat)

if (IRCCD_WITH_SSL)
	target_link_libraries(libirccd-static OpenSSL::SSL OpenSSL::Crypto)
endif ()

# pkg-config file
if (APPLE)
	set(EXTRA_LIBS "-undefined dynamic_lookup")
endif ()

configure_file(
	${libirccd_SOURCE_DIR}/irccd.pc
	${libirccd_BINARY_DIR}/irccd.pc
)

write_basic_package_version_file(${libirccd_BINARY_DIR}/IrccdConfigVersion.cmake
	VERSION ${IRCCD_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(TARGETS libirccd EXPORT IrccdTargets)
install(
	EXPORT IrccdTargets
	FILE IrccdTargets.cmake
	NAMESPACE
		irccd::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/irccd
)
install(
	FILES
		${CMAKE_SOURCE_DIR}/cmake/IrccdDefinePlugin.cmake
		${libirccd_SOURCE_DIR}/IrccdConfig.cmake
		${libirccd_BINARY_DIR}/IrccdConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/irccd
)

install(
	FILES
		${HEADERS}
		${libirccd_BINARY_DIR}/irccd/config.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/irccd
)

install(
	FILES ${libirccd_BINARY_DIR}/irccd.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

source_group(TREE ${libirccd_SOURCE_DIR} FILES ${SOURCES})

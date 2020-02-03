#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

find_package(Boost 1.60 REQUIRED QUIET COMPONENTS date_time filesystem system)

set(
	SOURCES
	${libirccd_SOURCE_DIR}/irccd/acceptor.hpp
	${libirccd_SOURCE_DIR}/irccd/config.cpp
	${libirccd_SOURCE_DIR}/irccd/config.hpp
	${libirccd_SOURCE_DIR}/irccd/connector.hpp
	${libirccd_SOURCE_DIR}/irccd/core.hpp
	${libirccd_SOURCE_DIR}/irccd/fs_util.cpp
	${libirccd_SOURCE_DIR}/irccd/fs_util.hpp
	${libirccd_SOURCE_DIR}/irccd/ini.cpp
	${libirccd_SOURCE_DIR}/irccd/ini.hpp
	${libirccd_SOURCE_DIR}/irccd/ini_util.hpp
	${libirccd_SOURCE_DIR}/irccd/json_util.cpp
	${libirccd_SOURCE_DIR}/irccd/json_util.hpp
	${libirccd_SOURCE_DIR}/irccd/options.hpp
	${libirccd_SOURCE_DIR}/irccd/stream.hpp
	${libirccd_SOURCE_DIR}/irccd/string_util.cpp
	${libirccd_SOURCE_DIR}/irccd/string_util.hpp
	${libirccd_SOURCE_DIR}/irccd/system.cpp
	${libirccd_SOURCE_DIR}/irccd/system.hpp
	${libirccd_SOURCE_DIR}/irccd/xdg.hpp
)

set(
	LIBRARIES
	${CMAKE_DL_LIBS}
	libjson
	Threads::Threads
	Boost::disable_autolinking
	Boost::dynamic_linking
	Boost::date_time
	Boost::filesystem
	Boost::system
)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
	list(APPEND LIBRARIES mswsock shlwapi ws2_32)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	#
	# Disable epoll in boost until it get fixed:
	# https://github.com/boostorg/asio/issues/150
	#
	list(APPEND FLAGS "BOOST_ASIO_DISABLE_EPOLL")
elseif (APPLE)
	list(APPEND LIBRARIES resolv)
endif ()

if (IRCCD_HAVE_SSL)
	list(APPEND LIBRARIES OpenSSL::Crypto OpenSSL::SSL)
endif ()

irccd_define_library(
	TARGET libirccd
	EXPORT
	FLAGS ${FLAGS}
	HEADERS ${libirccd_SOURCE_DIR}/irccd/
	SOURCES ${SOURCES}
	LIBRARIES ${LIBRARIES}
	PUBLIC_INCLUDES
		$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
		$<BUILD_INTERFACE:${libirccd_SOURCE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

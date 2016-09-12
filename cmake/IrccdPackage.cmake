#
# CPackConfing.cmake -- CMake build system for irccd
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

set(CPACK_SOURCE_PACKAGE_FILE_NAME "irccd-${IRCCD_VERSION}")
set(CPACK_SOURCE_GENERATOR TXZ ZIP)
set(CPACK_SOURCE_IGNORE_FILES "/[.]hg" "/CMakeLists[.]txt[.]user")

set(CPACK_PACKAGE_NAME "irccd")
set(CPACK_PACKAGE_VENDOR "malikania")
set(CPACK_PACKAGE_VERSION_MAJOR ${IRCCD_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${IRCCD_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${IRCCD_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.md)

#
# Installer hierarchy.
# -------------------------------------------------------------------
#
#   -- Applications         (Group: Applications)
#       | -- irccd          (Component: irccd)
#       | -- irccdctl       (Component: irccdctl)
#   -- Development          (Group: Development)
#       | -- C++ Headers    (Component: headers)
#   -- Plugins              (Group: Plugins)
#       | -- *              (Component: *)
#
# Replace * with the appropriate plugin name.
#

if (WIN32)
    set(CPACK_GENERATOR "NSIS")
endif ()

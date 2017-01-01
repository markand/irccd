#
# Options.cmake -- CMake build system for irccd
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
# Options that controls the build:
#
# WITH_SSL              Enable OpenSSL (default: on)
# WITH_JS               Enable JavaScript (default: on)
# WITH_TESTS            Enable unit testing (default: off)
# WITH_SYSTEMD          Install systemd service (default: on for Linux)
# WITH_DOCS             Enable building of all documentation (default: on)
# WITH_DOXYGEN          Enable internal irccd documentation (default: on)
# WITH_HTML             Enable HTML documentation
# WITH_MAN              Install manpages (default: on, off for Windows)
# WITH_PKGCONFIG        Install pkg-config files (default: on, off for Windows (except MinGW))
# WITH_PLUGIN_<NAME>    Enable or disable the specified plugin (default: on)
#
# Note: the option() commands for WITH_PLUGIN_<name> variables are defined automatically from the IRCCD_PLUGINS
# list.
#

#
# Options that controls both installations and the irccd runtime:
#
# WITH_BINDIR           Binary directory for irccd, irccdctl
# WITH_PLUGINDIR        Path where plugins must be installed
# WITH_NPLUGINDIR       Path where native plugins must be installed
# WITH_DOCDIR           Path where to install documentation
# WITH_MANDIR           Path where to install manuals
# WITH_CONFDIR          Path where to search configuration files
# WITH_CACHEDIR         Path where to store temporary files
# WITH_PKGCONFIGDIR     Path where to install pkg-config files
# WITH_SYSTEMDDIR       Path where to install systemd unit file
#

#
# Options for unit tests only:
#
# WITH_TEST_IRCHOST     Which IRC server to use for tests (default: 127.0.0.1)
# WITH_TEST_IRCPORT     Which IRC server port to use for tests (default: 6667)
#

# Manual pages on Windows are pretty useless.
if (WIN32)
    set(DEFAULT_MAN "No")
else ()
    set(DEFAULT_MAN "Yes")
endif ()

# Systemd unit file
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(DEFAULT_SYSTEMD "Yes")
else ()
    set(DEFAULT_SYSTEMD "No")
endif ()

# pkg-config is only relevant on UNIX or MinGW
if (MINGW OR UNIX)
    set(DEFAULT_PKGCONFIG "Yes")
else ()
    set(DEFAULT_PKGCONFIG "No")
endif ()

option(WITH_SSL "Enable SSL" On)
option(WITH_JS "Enable embedded Duktape" On)
option(WITH_TESTS "Enable unit testing" Off)
option(WITH_SYSTEMD "Install systemd service" ${DEFAULT_SYSTEMD})
option(WITH_DOCS "Enable building of all documentation" On)
option(WITH_HTML "Enable building of HTML documentation" On)
option(WITH_DOXYGEN "Enable doxygen" Off)
option(WITH_MAN "Install man pages" ${DEFAULT_MAN})
option(WITH_PKGCONFIG "Enable pkg-config file" ${DEFAULT_PKGCONFIG})

set(WITH_TEST_IRCHOST "127.0.0.1" CACHE STRING "IRC host for tests")
set(WITH_TEST_IRCPORT 6667 CACHE STRING "IRC port for test")

#
# Installation paths.
# -------------------------------------------------------------------
#

set(WITH_BINDIR "bin" CACHE STRING "Binary directory")
set(WITH_MANDIR "share/man" CACHE STRING "Man directory")
set(WITH_CONFDIR "etc" CACHE STRING "Configuration directory")
set(WITH_CMAKEDIR "lib/cmake" CACHE STRING "Directory for CMake modules")
set(WITH_PKGCONFIGDIR "lib/pkgconfig" CACHE STRING "Directory for pkg-config file")
set(WITH_SYSTEMDDIR "/usr/lib/systemd/system" CACHE STRING "Absolute path where to install systemd files")

#
# On Windows, we install the applcation like C:/Program Files/irccd so do not append irccd to the
# directories again.
#
if (WIN32)
    set(WITH_DATADIR "share" CACHE STRING "Data directory")
    set(WITH_CACHEDIR "var" CACHE STRING "Temporary files directory")
    set(WITH_PLUGINDIR "share/plugins" CACHE STRING "Module prefix where to install")
    set(WITH_NPLUGINDIR "lib/irccd/plugins" CACHE STRING "Directory for native plugins")
    set(WITH_DOCDIR "share/doc" CACHE STRING "Documentation directory")
else ()
    set(WITH_DATADIR "share/irccd" CACHE STRING "Data directory")
    set(WITH_CACHEDIR "var/irccd" CACHE STRING "Temporary files directory")
    set(WITH_PLUGINDIR "share/irccd/plugins" CACHE STRING "Module prefix where to install")
    set(WITH_NPLUGINDIR "lib/irccd/plugins" CACHE STRING "Directory for native plugins")
    set(WITH_DOCDIR "share/doc/irccd" CACHE STRING "Documentation directory")
endif ()

#
# Check if any of these path is absolute and raise an error if true.
#
foreach (d WITH_BINDIR WITH_CACHEDIR WITH_DATADIR WITH_CONFDIR WITH_PLUGINDIR)
    if (IS_ABSOLUTE ${${d}})
        message(FATAL_ERROR "${d} can not be absolute (${${d}} given)")
    endif ()
endforeach ()

#
# Internal dependencies.
# -------------------------------------------------------------------
#

if (WITH_JS)
    add_subdirectory(extern/duktape)
    set(WITH_JS_MSG "Yes")
else ()
    set(WITH_JS_MSG "No")
endif ()

if (WITH_TESTS)
    add_subdirectory(extern/gtest)
    set(WITH_TESTS_MSG "Yes")
else ()
    set(WITH_TESTS_MSG "No")
endif ()

#
# External dependencies.
# -------------------------------------------------------------------
#

find_package(Doxygen)
find_package(Pandoc)
find_package(OpenSSL)

if (NOT WITH_DOCS)
    set(WITH_HTML FALSE)
    set(WITH_DOXYGEN FALSE)
    set(WITH_MAN FALSE)
endif ()

if (WITH_SSL)
    if (OPENSSL_FOUND)
        set(WITH_SSL_MSG "Yes")
    else ()
        set(WITH_SSL_MSG "No (OpenSSL not found)")
        set(WITH_SSL FALSE)
    endif ()
else()
    set(WITH_SSL_MSG "No (disabled by user)")
endif ()

if (WITH_DOXYGEN)
    if (DOXYGEN_FOUND)
        set(WITH_DOXYGEN_MSG "Yes")
    else ()
        set(WITH_DOXYGEN_MSG "No (doxygen not found)")
        set(WITH_DOXYGEN FALSE)
    endif ()
else ()
    set(WITH_DOXYGEN_MSG "No (disabled by user)")
endif ()

if (WITH_HTML)
    if (Pandoc_FOUND)
        set(WITH_HTML_MSG "Yes")
    else ()
        set(WITH_HTML_MSG "No (pandoc not found)")
        set(WITH_HTML FALSE)
    endif ()
else ()
    set(WITH_HTML_MSG "No (disabled by user)")
    set(WITH_HTML FALSE)
endif ()

#
# Determine if allowed to package.
# -------------------------------------------------------------------
#
# Do not move this section because irccd's CMake functions requires the IRCCD_PACKAGE value.
#

set(IRCCD_PACKAGE FALSE)

if (NOT WITH_HTML)
    set(IRCCD_PACKAGE_MSG "No (HTML documentation disabled)")
else ()
    # Now check that a plugin has not been disabled.
    set(IRCCD_PACKAGE TRUE)
    set(IRCCD_PACKAGE_MSG "Yes")

    foreach (plugin ${IRCCD_PLUGINS})
        string(TOUPPER ${plugin} optname)

        if (NOT WITH_PLUGIN_${optname})
            set(IRCCD_PACKAGE FALSE)
            set(IRCCD_PACKAGE_MSG "No (Plugin ${plugin} disabled)")
        endif ()
    endforeach ()
endif ()

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
# WITH_VERA             Enable style checking using vera (default: on)
#
# Note: the option() commands for WITH_PLUGIN_<name> variables are defined automatically from the IRCCD_PLUGINS
# list.
#

#
# Options that controls both installations and the irccd runtime:
#
# WITH_BINDIR           Binary directory for irccd, irccdctl
# WITH_CACHEDIR         Path where to store temporary files
# WITH_CMAKEDIR         Path where to install CMake configuration files
# WITH_DATADIR          Path for data files
# WITH_DOCDIR           Path where to install documentation
# WITH_MANDIR           Path where to install manuals
# WITH_PKGCONFIGDIR     Path where to install pkg-config files
# WITH_PLUGINDIR        Path where plugins must be installed
# WITH_SYSCONFDIR       Path where to install configuration files
# WITH_SYSTEMDDIR       Path where to install systemd unit file
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
option(WITH_VERA "Enable vera++" On)

#
# Installation paths.
# -------------------------------------------------------------------
#

set(WITH_BINDIR "bin" CACHE STRING "Binary directory")
set(WITH_CACHEDIR "var/cache/irccd" CACHE STRING "Cache directory")
set(WITH_CMAKEDIR "lib/cmake" CACHE STRING "Directory for CMake modules")
set(WITH_DATADIR "share/irccd" CACHE STRING "Directory for additional data")
set(WITH_DOCDIR "share/doc/irccd" CACHE STRING "Documentation directory")
set(WITH_MANDIR "share/man" CACHE STRING "Man directory")
set(WITH_PKGCONFIGDIR "lib/pkgconfig" CACHE STRING "Directory for pkg-config file")
set(WITH_PLUGINDIR "libexec/irccd/plugins" CACHE STRING "Module prefix where to install")
set(WITH_SYSCONFDIR "etc" CACHE STRING "Configuration directory")
set(WITH_SYSTEMDDIR "/usr/lib/systemd/system" CACHE STRING "Absolute path where to install systemd files")

#
# Internal dependencies.
# -------------------------------------------------------------------
#

if (WITH_JS)
    add_subdirectory(extern/duktape)
    set(HAVE_JS On)
    set(WITH_JS_MSG "Yes")
else ()
    set(WITH_JS_MSG "No")
endif ()

if (WITH_TESTS)
    set(WITH_TESTS_MSG "Yes")
else ()
    set(WITH_TESTS_MSG "No")
endif ()

#
# External dependencies.
# -------------------------------------------------------------------
#

find_package(Doxygen)
find_package(OpenSSL)
find_package(Pandoc)
find_package(TCL)

if (NOT WITH_DOCS)
    set(WITH_HTML FALSE)
    set(WITH_DOXYGEN FALSE)
    set(WITH_MAN FALSE)
endif ()

if (WITH_SSL)
    if (OPENSSL_FOUND)
        set(HAVE_SSL On)
        set(WITH_SSL_MSG "Yes")
    else ()
        set(WITH_SSL_MSG "No (OpenSSL not found)")
    endif ()
else()
    set(WITH_SSL_MSG "No (disabled by user)")
endif ()

if (WITH_DOXYGEN)
    if (DOXYGEN_FOUND)
        set(HAVE_DOXYGEN On)
        set(WITH_DOXYGEN_MSG "Yes")
    else ()
        set(WITH_DOXYGEN_MSG "No (doxygen not found)")
    endif ()
else ()
    set(WITH_DOXYGEN_MSG "No (disabled by user)")
endif ()

if (WITH_HTML)
    if (Pandoc_FOUND)
        set(HAVE_HTML On)
        set(WITH_HTML_MSG "Yes")
    else ()
        set(WITH_HTML_MSG "No (pandoc not found)")
    endif ()
else ()
    set(WITH_HTML_MSG "No (disabled by user)")
endif ()

if (WITH_VERA)
    if (TCL_FOUND)
        add_subdirectory(extern/vera)
        set(HAVE_VERA On)
        set(WITH_VERA_MSG "Yes")
    else ()
        set(WITH_VERA_MSG "No (TCL not found)")
    endif ()
else ()
    set(WITH_VERA_MSG "No (disabled by user)")
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

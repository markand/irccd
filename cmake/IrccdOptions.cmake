#
# Options.cmake -- CMake build system for irccd
#
# Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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
# IRCCD_WITH_DOCS           Enable building of all documentation (default: on)
# IRCCD_WITH_DOXYGEN        Enable internal irccd documentation (default: on)
# IRCCD_WITH_HTML           Enable HTML documentation
# IRCCD_WITH_JS             Enable JavaScript (default: on)
# IRCCD_WITH_LIBEDIT        Enable libedit support (default: on)
# IRCCD_WITH_MAN            Install manpages (default: on, off for Windows)
# IRCCD_WITH_PKGCONFIG      Install pkg-config files (default: on, off for Windows (except MinGW))
# IRCCD_WITH_PLUGIN_<NAME>  Enable or disable the specified plugin (default: on)
# IRCCD_WITH_SSL            Enable OpenSSL (default: on)
# IRCCD_WITH_SYSTEMD        Install systemd service (default: on for Linux)
# IRCCD_WITH_TESTS          Enable unit testing (default: off)
# IRCCD_WITH_VERA           Enable style checking using vera (default: on)
#
# Note: the option() commands for IRCCD_WITH_PLUGIN_<name> variables are
# defined automatically from the IRCCD_PLUGINS list.
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

option(IRCCD_WITH_DOCS "Enable building of all documentation" On)
option(IRCCD_WITH_DOXYGEN "Enable doxygen" Off)
option(IRCCD_WITH_HTML "Enable building of HTML documentation" On)
option(IRCCD_WITH_JS "Enable embedded Duktape" On)
option(IRCCD_WITH_LIBEDIT "Enable libedit support" On)
option(IRCCD_WITH_MAN "Install man pages" ${DEFAULT_MAN})
option(IRCCD_WITH_PKGCONFIG "Enable pkg-config file" ${DEFAULT_PKGCONFIG})
option(IRCCD_WITH_SSL "Enable SSL" On)
option(IRCCD_WITH_SYSTEMD "Install systemd service" ${DEFAULT_SYSTEMD})
option(IRCCD_WITH_TESTS "Enable unit testing" Off)
option(IRCCD_WITH_VERA "Enable vera++" On)

#
# Internal dependencies.
# -------------------------------------------------------------------
#

if (IRCCD_WITH_JS)
	add_subdirectory(extern/duktape)
	set(IRCCD_HAVE_JS On)
	set(IRCCD_WITH_JS_MSG "Yes")
else ()
	set(IRCCD_WITH_JS_MSG "No")
endif ()

if (IRCCD_WITH_TESTS)
	set(IRCCD_WITH_TESTS_MSG "Yes")
else ()
	set(IRCCD_WITH_TESTS_MSG "No")
endif ()

#
# External dependencies.
# -------------------------------------------------------------------
#

find_package(Doxygen)
find_package(OpenSSL)
find_package(Pandoc)
find_package(TCL QUIET)
find_package(Editline)

if (NOT IRCCD_WITH_DOCS)
	set(IRCCD_WITH_HTML FALSE)
	set(IRCCD_WITH_DOXYGEN FALSE)
	set(IRCCD_WITH_MAN FALSE)
endif ()

if (IRCCD_WITH_LIBEDIT)
	if (Editline_FOUND)
		set(IRCCD_HAVE_LIBEDIT On)
		set(IRCCD_WITH_LIBEDIT_MSG "Yes")
	else ()
		set(IRCCD_WITH_LIBEDIT_MSG "No (libedit not found)")
	endif ()
else ()
	set(IRCCD_WITH_LIBEDIT_MSG "No (disabled by user)")
endif ()

if (IRCCD_WITH_SSL)
	if (OPENSSL_FOUND)
		set(IRCCD_HAVE_SSL On)
		set(IRCCD_WITH_SSL_MSG "Yes")
	else ()
		set(IRCCD_WITH_SSL_MSG "No (OpenSSL not found)")
	endif ()
else()
	set(IRCCD_WITH_SSL_MSG "No (disabled by user)")
endif ()

if (IRCCD_WITH_DOXYGEN)
	if (DOXYGEN_FOUND)
		set(IRCCD_HAVE_DOXYGEN On)
		set(IRCCD_WITH_DOXYGEN_MSG "Yes")
	else ()
		set(IRCCD_WITH_DOXYGEN_MSG "No (doxygen not found)")
	endif ()
else ()
	set(IRCCD_WITH_DOXYGEN_MSG "No (disabled by user)")
endif ()

if (IRCCD_WITH_HTML)
	if (Pandoc_FOUND)
		set(IRCCD_HAVE_HTML On)
		set(IRCCD_WITH_HTML_MSG "Yes")
	else ()
		set(IRCCD_WITH_HTML_MSG "No (pandoc not found)")
	endif ()
else ()
	set(IRCCD_WITH_HTML_MSG "No (disabled by user)")
endif ()

if (IRCCD_WITH_VERA)
	if (TCL_FOUND)
		add_subdirectory(extern/vera)
		set(IRCCD_HAVE_VERA On)
		set(IRCCD_WITH_VERA_MSG "Yes")
	else ()
		set(IRCCD_WITH_VERA_MSG "No (TCL not found)")
	endif ()
else ()
	set(IRCCD_WITH_VERA_MSG "No (disabled by user)")
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

		if (NOT IRCCD_WITH_PLUGIN_${optname})
			set(IRCCD_PACKAGE FALSE)
			set(IRCCD_PACKAGE_MSG "No (Plugin ${plugin} disabled)")
		endif ()
	endforeach ()
endif ()

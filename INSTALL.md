IRC Client Daemon INSTALL
=========================

This guide will help you to install irccd.

Requirements
------------

Build dependencies:

- C99 and few features from C11 (stdatomics.h, stdnoreturn.h).
- [Bison][] and [Flex][]: for configuration files.
- [CMake][]: the cross platform build utility.

Optional runtime dependencies:

- [OpenSSL][]: Used for SSL connections to IRC servers (recommended).
- [CURL][]: Required for the links plugin.

Basic installation
------------------

This is the quick way to install irccd.

	tar xvzf irccd-x.y.z-tar.xz
	cmake -S irccd-x.y.z -B build
	cmake --build build --target all
	sudo cmake --build build --target install

### Disabling plugins

For every plugin, an option `IRCCD_WITH_PLUGIN_<NAME>` is presented and set to
on by default. Substitute `<NAME>` with the plugin uppercase name (e.g. `ASK`).

	cmake build -DIRCCD_WITH_PLUGIN_ASK=Off

### Systemd service

The systemd service file is installed by default if the machine is running
systemd, it's possible to force the option using `IRCCD_WITH_SYSTEMD` to `On` or
`Off`.

	cmake build -DIRCCD_WITH_SYSTEMD=Off

### Other options

The following options are available:

- `IRCCD_WITH_MAN`: installation of manpages.
- `IRCCD_WITH_JS`: Javascript support (and all Javascript plugins).
- `IRCCD_WITH_SSL`: OpenSSL support.
- `IRCCD_WITH_TESTS`: building of tests.
- `IRCCD_WITH_EXAMPLES`: plugin templates.
- `IRCCD_WITH_PKGCONFIG`: pkg-config file installation.
- `IRCCD_WITH_PKGCONFIGDIR`: path to pkg-config directory installation.

The following standard CMake options control installation directories.

- `CMAKE_INSTALL_PREFIX`: root directory for installing (default: /usr/local).
- `CMAKE_INSTALL_BINDIR`: binaries (default: ${CMAKE_INSTALL_PREFIX}/bin).
- `CMAKE_INSTALL_SYSCONFDIR`: config files (default: ${CMAKE_INSTALL_PREFIX}/etc).
- `CMAKE_INSTALL_LIBDIR`: libraries (default: ${CMAKE_INSTALL_PREFIX}/lib).
- `CMAKE_INSTALL_INCLUDEDIR`: header files (default: ${CMAKE_INSTALL_PREFIX}/include).
- `CMAKE_INSTALL_SHAREDIR`: data files (default: ${CMAKE_INSTALL_PREFIX}/share).
- `CMAKE_INSTALL_MANDIR`: path to manual pages (default: ${CMAKE_INSTALL_PREFIX}/share/man).

[Bison]: https://www.gnu.org/software/bison
[CMake]: http://www.cmake.org
[CURL]: https://curl.se
[Flex]: https://github.com/westes/flex
[OpenSSL]: http://openssl.org

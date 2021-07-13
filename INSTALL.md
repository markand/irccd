IRC Client Daemon INSTALL
=========================

This guide will help you to install irccd.

Requirements
------------

Build dependencies:

- C99 and few features from C11 (stdatomics.h, stdnoreturn.h).
- [Bison][] and [Flex][]: For configuration files.
- [GNU Make][]: The GNU make utility.

Optional runtime dependencies:

- [OpenSSL][]: Used for SSL connections to IRC servers (recommended).
- [CURL][]: Required for the links plugin.

Basic installation
------------------

This is the quick way to install irccd.

    tar xvzf irccd-x.y.z-tar.xz
    cd irccd-x.y.z
    make
    sudo make install

### Installing plugins

And to install plugins you must build and use `plugins` and `install-plugins`
targets:

    make plugins
    sudo make install-plugins

Alternatively you can build and install plugins manually using `plugin-NAME` and
`install-plugin-NAME` to install only wanted plugins.

Example:

    make plugin-ask plugin-links
    sudo make install-plugin-ask plugin-links

### Systemd service

A systemd service is available through the `install-systemd` target. The service
file is named `irccd.service`.

Options
-------

The following options are available at build time (make sure to run `make clean`
when you change options).

- `SSL`: set to 1 or 0 to enable/disable OpenSSL (default: 1).
- `JS`: set to 1 or 0 to disable Javascript (default: 1).
- `DEBUG`: set to 1 or 0 to build with debug symbols and optimizations disabled
  (default: 0).
- `USER`: user to use for transport and init scripts (default: nobody).
- `GROUP`: group to use for transport and init scripts (default: nobody).

You can tweak the installation directories by changing the following variables
(note: all paths must be absolute):

- `PREFIX`: root directory for installing (default: /usr/local).
- `BINDIR`: binaries (default: ${PREFIX}/bin).
- `ETCDIR`: config files (default: ${PREFIX}/etc).
- `LIBDIR`: libraries (default: ${PREFIX}/lib).
- `INCDIR`: header files (default: ${PREFIX}/include).
- `SHAREDIR`: data files (default: ${PREFIX}/share).
- `MANDIR`: path to manual pages (default: ${PREFIX}/share/man).
- `VARDIR`: local cache files (default: ${PREFIX}/var).

For any options, make sure to specify them for any make targets (e.g. `install`)
because they can be translated at install time too.

[Bison]: https://www.gnu.org/software/bison
[GNU Make]: http://www.cmake.org
[CURL]: https://curl.se
[Flex]: https://github.com/westes/flex
[OpenSSL]: http://openssl.org
[libbsd]: https://libbsd.freedesktop.org/wiki

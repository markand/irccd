IRC Client Daemon INSTALL
=========================

This guide will help you to install irccd.

Requirements
------------

Build dependencies:

- C23 compatible compiler
- [GNU make][]

Optional runtime dependencies:

- [LibreTLS][]: Used for SSL connections to IRC servers (recommended).
- [CURL][]: Required for the Http Javascript API (and links plugin)..

Basic installation
------------------

This is the quick way to install irccd.

	tar -xvf irccd-x.y.z-tar.xz
	make
	doas make install

### Disabling plugins

For every plugin, an option `PLUGIN_<NAME>` is available and set to on by
default. Substitute `<NAME>` with the plugin uppercase name (e.g. `ASK`).

	make PLUGIN_ASK=0

### Systemd service

The systemd service file is installed by default if the machine is running
systemd, it's possible to force the option using `SYSTEMD` to `1` or `0`.

	make SYSTEMD=0

### Other options

The following options are available:

- `MAN`: installation of manpages.
- `JS`: Javascript support (and all Javascript plugins).
- `SSL`: OpenSSL support.
- `TESTS`: building of tests.
- `HTTP`: enable HTTP support (with libcurl dependency).

The following make variables define installation paths:

- `PREFIX`: root directory for installing (default: /usr/local).
- `BINDIR`: binaries (default: $(PREFIX)/bin).
- `DATADIR`: sample files (default: $(PREFIX)/share).
- `SYSCONFDIR`: config files (default: $(PREFIX)/etc).
- `LIBDIR`: libraries (default: $(PREFIX)/lib).
- `INCLUDEDIR`: header files (default: $(PREFIX)/include).
- `SHAREDIR`: data files (default: $(PREFIX)/share).
- `MANDIR`: path to manual pages (default: $(PREFIX)/share/man).
- `VARDIR`: local state files (default: $(PREFIX)/var).

[GNU Make]: https://www.gnu.org/software/make
[CURL]: https://curl.se
[LibreTLS]: https://git.causal.agency/libretls

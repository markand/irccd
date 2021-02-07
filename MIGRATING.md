IRC Client Daemon MIGRATING
===========================

This document is a small guide to help you migrating to a next major version.

Migrating from 3.x to 4.x
=========================

Irccd
-----

- The `irccd.conf` is now using a custom syntax.
- There are no longer TCP/IP transports. SSL support for transports has been
  removed too. Only clear UNIX socket are available, use file permissions
  instead of a password.

Irccdctl
--------

- There is no longer configuration file because it now use a plain UNIX socket
  to */tmp/irccd.sock* by default (may be changed through the `-s` option).
- Aliases have been removed, please use shell scripts or aliases instead.
- The `watch` command no longer produce JSON output but only the original
  "human" format but may be used for scripts as it is honored through the
  semantic versioning.

Platform support
----------------

Windows support is now completely obsolescent because the code make excessive
use of POSIX APIs.

Network API
-----------

The network protocol no longer uses JSON but a plain text syntax with one line
per command.

Example:

    SERVER-MESSAGE freenode #staff hello world
    SERVER-LIST

Javascript API
--------------

### Module File

- The method `File.readline` is no longer marked as slow.
- Methods `File.lines`, `File.read`, `File.readline` and `File.seek`,  now throw
  an exception if the file was closed.

### Module Chrono

- The `Chrono.elapsed()` method is now a property named `elapsed`.
- All methods have been removed except `Chrono.reset`.

### Module Util

- The method `Util.ticks` as been removed.
- The method `Util.cut` now throws a `RangeError` exception if the number of
  lines exceed `maxl` argument instead of returning null.

### Module Server

- The property `channels` in the object returned from `Server.info` is now an
  array of objects which also contain a list of nicknames present in the
  channel.
- The property `channels` in the object for the `Server` constructor now takes
  an array of objects containing two properties each: `name` and `password`
  which must be string (password is optional).

Migrating from 2.x to 3.x
=========================

Irccd
-----

- Long options have been removed.
- The option `reconnect-tries` has been removed from `[server]` section, use
  `auto-reconnect` boolean option instead,
- The option `reconnect-timeout` has been renamed to `auto-reconnect-delay`.
- The section `[identity]` has been removed, instead move those values inside
  each server in their `[server]` section.
- The section `[format]` and their respective plugin counterparts are renamed to
  `[templates]`.

Irccdctl
--------

- Long options have been removed.
- The functions `server-cnotice` and `server-cmode` have been removed, use
  `server-notice` and `server-mode` instead,
- The option `connect.host` has been renamed to `connect.hostname`,
- The output style has been unified,
- Options `-S` in server-connect have been removed, also
  the port option is now specified with `-p` instead of a positional argument.
- Connection options are now order dependant and must be set before the command
  name.

Plugins
-------

### Logger

- The keyword `source` has been removed and replaced by `channel`,
- The keyword `origin` has been added,
- Formats `cnotice`, `cmode`, `query` have been removed.
- The option `path` has been renamed to `file`.

Network API
-----------

- The requests `server-cnotice` and `server-cmode` have been removed, use
  `server-notice` and `server-mode` instead,
- The request `server-mode` command requires a new argument `channel`.
- The property `host` in request `server-connect` has been renamed to
  `hostname`,
- The request `server-info` sends `hostname` property instead of `host`,
- The event `onWhois` sends `hostname` property instead of `host`,

CMake options
-------------

- All options are now starting with `IRCCD_` for better compatibility with
  external projects,
- CMake now use GNUInstallDirs module to specify installation paths, all
  IRCCD\_WITH\_ options have been replaced by CMAKE\_INSTALL\_ equivalents.

Directories
-----------

- The default plugins path has been changed from **share/irccd/plugins** to
  **lib(arch)/irccd**.

Javascript API
--------------

### Events

- The events `onChannelMode` and `onChannelNotice` have been removed, plugins
  must use `Server.isSelf(target)` to determine a channel/private message,
- The event `onNotice` takes a new `channel` argument,
- The event `onMode` takes new `channel`, `limit`, `user`, `mask` arguments,
- The object information in `onWhois` event now has `hostname` property instead
  of `host`.

### Module Chrono

- The module `ElapsedTimer` has been renamed to `Chrono`,
- The method `Chrono.restart` has been renamed to `Chrono.resume` to reduce
  ambiguity,
- The method `Chrono.reset` has been removed, just use `Chrono.start` instead
  when you want to start accumulate time again.

### Module Directory

- The property `Directory.count` has been removed.

### Module Server

- The methods `Server.cmode` and `Server.cnotice` have been removed, use
  `Server.mode` and `Server.notice` instead,
- The method `Server.mode` requires a new argument `channel`,
- The object returned in the method `Server.info` now has a `hostname` property
  instead of `host`.
- The property `host` in constructor `Server` has been renamed to
  `hostname`,
- The property `sslVerify` in `Server` constructor has been removed.

### Module Plugin

The following properties in `Irccd.Plugin` has been renamed:

- **cachePath** renamed to **paths.cache**,
- **configPath** renamed to **paths.config**,
- **dataPath** renamed to **paths.data**.

Note: these paths are no more automatically detected and set with the new
      `[paths]` and `[paths.<name>]` sections.

### Module Server

- The property `host` in the function `Irccd.Server` has been renamed to
  `hostname`.

### Module System

- The function `Irccd.System.name` has now well defined return value.

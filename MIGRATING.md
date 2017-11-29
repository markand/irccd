IRC Client Daemon MIGRATING
===========================

This document is a small guide to help you migrating to a next major version.

Migrating from 2.x to 3.x
-------------------------

### Irccdctl

  - The functions `server-cnotice` and `server-cmode` have been removed, use
    `server-notice` and `server-mode` instead.

### Network API

  - The requests `server-cnotice` and `server-cmode` have been removed, use
    `server-notice` and `server-mode` instead,
  - The request `server-mode` command requires a new argument `channel`.

### CMake options

  - `WITH_CONFDIR` has been renamed to `WITH_SYSCONFDIR`.

### Paths

  - The default plugins path has been changed from **share/irccd/plugins** to
    **libexec/irccd/plugins**.

### Javascript API

#### Events

  - The events `onChannelMode` and `onChannelNotice` have been removed, plugins
    must use `Server.isSelf(target)` to determine a channel/private message.

#### Module Server

  - The methods `Server.cmode` and `Server.cnotice` have been removed, use
    `Server.mode` and `Server.notice` instead,
  - The method `Server.mode` requires a new argument `channel`.

#### Module ElapsedTimer

  - The method ElapsedTimer.reset has been removed, just use `start` instead
    when you want to accumulate time.

#### Module Directory

  - The property `Directory.count` has been removed.

### Module Plugin

The following properties in `Irccd.Plugin` has been renamed:

  - **cachePath** renamed to **paths.cache**,
  - **configPath** renamed to **paths.config**,
  - **dataPath renamed** to **paths.data**.

Note: these paths are no more automatically detected and set with the new
      `[paths]` and `[paths.<name>]` sections.


IRC Client Daemon MIGRATING
===========================

This document is a small guide to help you migrating to a next major version.

Migrating from 2.x to 3.x
-------------------------

### CMake options

  - WITH_CONFDIR has been renamed to WITH_SYSCONFDIR.

### Paths

  - The default plugins path has been changed from **share/irccd/plugins** to
    **libexec/irccd/plugins**.

### Plugin configuration

The following properties in `Irccd.Plugin` has been renamed:

  - cachePath renamed to paths.cache,
  - configPath renamed to paths.config,
  - dataPath renamed to paths.data.

Note: these paths are no more automatically detected and set with the new
      `[paths]` and `[paths.<name>]` sections.

### Javascript API

#### Module ElapsedTimer

  - The method ElapsedTimer.reset has been removed, just use `start` instead
    when you want to accumulate time.

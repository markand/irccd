# Module Irccd.Plugin

This module let you manage plugins.

# Objects

The following properties are defined as read in the configuration file:

- config: the `[plugin.<name>]` section,
- paths: the `[paths.<name>]` section,
- format: the `[format.<name>]` section.

# Functions

- [info](Irccd.Plugin.info.html)
- [list](Irccd.Plugin.list.html)
- [load](Irccd.Plugin.load.html)
- [reload](Irccd.Plugin.reload.html)
- [unload](Irccd.Plugin.unload.html)

# Example

Assuming the configuration file is defined as following:

```ini
[plugin.xyz]
foo = true
baz = "hello"

[paths.xyz]
config = "/etc/xyz"
```

The `Irccd.Plugin.config` will have the following properties:

- foo: (string) set to "true",
- baz: (string) set to "hello".

The `Irccd.Plugin.paths` will have the following properties:

- cache: (string) set to the default cache directory,
- config: (string) set to "/etc/xyz",
- data: (string) set to the default data directory.
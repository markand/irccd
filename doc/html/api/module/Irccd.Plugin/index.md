---
module: Irccd.Plugin
js: true
summary: Plugin management and inspection.
---

## Usage

This module let you manage plugins.

## Objects

The following properties are defined as read in the configuration file:

  - **config**: the `[plugin.<name>]` section,
  - **paths**: the `[paths.<name>]` section,
  - **format**: the `[format.<name>]` section.

### Example

Assuming the configuration file is defined as following:

<div class="panel panel-info">
 <div class="panel-heading">~/.config/irccd/irccd.conf</div>
 <div class="panel-body">
````ini
[plugin.xyz]
foo = true
baz = "hello"

[paths.xyz]
config = "/etc/xyz"
````
 </div>
</div>

The `Irccd.Plugin.config` will have the following properties:

  - **foo**: (string) set to "true",
  - **baz**: (string) set to "hello".

The `Irccd.Plugin.paths` will have the following properties:

  - **cache**: (string) set to the default cache directory,
  - **config**: (string) set to "/etc/xyz",
  - **data**: (string) set to the default data directory.

## Functions

  - [info](Irccd.Plugin.info.html)
  - [list](Irccd.Plugin.list.html)
  - [load](Irccd.Plugin.load.html)
  - [reload](Irccd.Plugin.reload.html)
  - [unload](Irccd.Plugin.unload.html)

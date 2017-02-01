---
module: Irccd.Plugin
js: true
summary: Plugin management and inspection.
---

## Usage

This module let you manage plugins.

## Constants

The following properties are defined:

  - **cachePath**: (string) the path to the cache directory,
  - **configPath**: (string) the path to the configuration directory,
  - **dataPath**: (string) the path to the data directory.

## Configuration

An additional property `config` is defined with all options set in the appropriate `[plugin.<name>]` from the user
configuration file.

### Example

If the configuration file configures the plugin **xyz**:

<div class="panel panel-info">
 <div class="panel-heading">~/.config/irccd/irccd.conf</div>
 <div class="panel-body">
````ini
[plugin.xyz]
foo = true
baz = "hello"
````
 </div>
</div>

Then `Irccd.Plugin.config` will have the following properties:

  - **foo**: (string) set to "true",
  - **baz**: (string) set to "hello".

## Functions

  - [info](Irccd.Plugin.info.html)
  - [list](Irccd.Plugin.list.html)
  - [load](Irccd.Plugin.load.html)
  - [reload](Irccd.Plugin.reload.html)
  - [unload](Irccd.Plugin.unload.html)

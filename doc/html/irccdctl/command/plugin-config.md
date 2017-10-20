---
title: plugin-config
guide: yes
---

# plugin-config

Get or set a plugin configuration variable.

If both variable and value are provided, sets the plugin configuration to the
respective variable name and value.

If only variable is specified, shows its current value. Otherwise, list all
variables and their values.

## Usage

```nohighlight
$ irccdctl plugin-config plugin [variable] [value]
```

## Example

```nohighlight
$ irccdctl plugin-config ask
```

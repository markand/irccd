---
title: server-part
guide: yes
---

# server-part

Leave the specified channel, the reason is optional.

<div class="alert alert-warning" role="alert">
**Warning**: not all IRC servers support giving a reason to leave a channel, do
not specify it if this is a concern.
</div>

## Usage

```nohighlight
$ irccdctl server-part server channel [reason]
```

## Example

```nohighlight
$ irccdctl server-part freenode #staff
$ irccdctl server-part freenode #botwar "too noisy"
```

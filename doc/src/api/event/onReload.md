# Event onReload

Request to reload the plugin.

# Synopsis

```javascript
function onReload()
```

This function is called when irccd instance reload a plugin. Thus, there are no
IRC events that call this function.

This function does nothing in the irccd internals, it just calls a function that
you can use to reload some data. It does not delete anything.

If you want to fully unload a plugin, use `irccdctl plugin-unload` then
`irccdctl plugin-load`.

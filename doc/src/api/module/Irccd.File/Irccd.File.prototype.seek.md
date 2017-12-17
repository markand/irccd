# Method Irccd.File.prototype.seek

# Summary

Sets the position in the file.

# Synopsis

```javascript
File.prototype.seek(type, amount)
```

# Arguments

  - **type**: the type of setting
    (`Irccd.File.SeekSet`, `Irccd.File.SeekCur`, `Irccd.File.SeekSet`),
  - **amount**: the new offset.

# Throws

An [Irccd.SystemError](@baseurl@api/module/Irccd/index.html#types) on failures.

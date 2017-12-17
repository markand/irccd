# Function Irccd.System.popen

Wrapper for `popen(3)` if the function is available.

**Note**: this function is optional and may not be available on your system.

# Synopsis

```javascript
Irccd.System.popen(cmd, mode) /* optional */
```

# Arguments

  - **cmd**: the command to execute,
  - **mode**: the mode (e.g. `r`).

# Returns

An [Irccd.File](@baseurl@api/module/Irccd.File/index.html) object.

# Throws

An [Irccd.SystemError](@baseurl@api/module/Irccd/index.html#types) on failures.

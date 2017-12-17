# Function Irccd.Directory.mkdir

Create a directory specified by path. It will create needed subdirectories just
like you have invoked `mkdir -p`.

# Synopsis

```javascript
Irccd.Directory.mkdir(path, mode = 0700)
```

# Arguments

  - **path**: the path to the directory,
  - **mode**: the mode, not available on all platforms.

# Throws

Any exception on error.

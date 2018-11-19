# Function Irccd.File (constructor)

Open a file specified by path with the specified mode.

# Synopsis

```javascript
Irccd.File(path, mode)
```

# Arguments

- path: the path to the file,
- mode: the mode string.

The mode is the same as the `fopen(3)`, see the documentation of
[std::fopen(3)][fopen] for more information about the mode string.

# Throws

An [Irccd.SystemError](#{baseurl}api/module/Irccd/index.html#types) on failures.

[fopen]: http://en.cppreference.com/w/cpp/io/c/fopen

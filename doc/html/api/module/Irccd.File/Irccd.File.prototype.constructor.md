---
method: constructor
js: true
summary: "Open a file specified by path with the specified mode."
synopsis: "Irccd.File(path, mode) /* constructor */"
arguments:
  - "**path**: the path to the file,"
  - "**mode**: the mode string."
throws: "An [Irccd.SystemError](@baseurl@api/module/Irccd/index.html#types) on failures."
---

## Mode

The mode is the same as the `fopen(3)`, see the documentation of [std::fopen(3)][fopen] for more information
about the mode string.

[fopen]: http://en.cppreference.com/w/cpp/io/c/fopen

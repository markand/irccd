---
function: find
js: true
summary: "Find an entry by a pattern or a regular expression."
synopsis: "Irccd.Directory.find(path, pattern, recursive)"
arguments:
  - "**path**: the base path,"
  - "**pattern**: the regular expression or file name,"
  - "**recursive**: set to true to search recursively (Optional, default: false)."
returns: "The path to the file or undefined if not found."
throws:
  - "Any exception on error."
---
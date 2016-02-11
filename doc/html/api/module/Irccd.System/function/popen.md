---
function: popen
summary: "Wrapper for `popen(3)` if the function is available."
synopsis: "Irccd.System.popen(cmd, mode) /* optional */"
arguments:
  - "cmd, the command to execute,"
  - "mode, the mode (e.g. `r`)."
returns: "An [Irccd.File](@baseurl@/api/module/Irccd.File/index.html) object."
throws: "Irccd.SystemError on failures."
---

## Remarks

<div class="alert alert-warning" role="alert">
**Warning**: this function is optional and may not be available on your system.
</div>

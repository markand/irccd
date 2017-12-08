---
function: cut
js: true
summary: "Cut a piece of data into several lines."
synopsis: "lines = Irccd.Util.cut(data, maxc, maxl)"
arguments:
  - "**data**: a string or an array of strings,"
  - "**maxc**: max number of colums (Optional, default: 72),"
  - "**maxl**: max number of lines (Optional, default: undefined)."
returns: "A list of strings ready to be sent or undefined if the data is too big."
throws:
  - "**RangeError** if maxl or maxc are negative numbers,"
  - "**RangeError** if one word length was bigger than maxc,"
  - "**TypeError** if data is not a string or a list of strings."
---

The argument data is a string or a list of strings. In any case, all strings
are first splitted by spaces and trimmed. This ensure that useless
whitespaces are discarded.

The argument maxc controls the maximum of characters allowed per line, it can
be a positive integer. If undefined is given, a default of 72 is used.

The argument maxl controls the maximum of lines allowed. It can be a positive
integer or undefined for an infinite list.

If maxl is used as a limit and the data can not fit within the bounds,
undefined is returned.

An empty list may be returned if empty strings were found.

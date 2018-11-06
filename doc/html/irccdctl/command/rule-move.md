---
title: rule-move
guide: yes
---

# rule-move

Move a rule from the given source at the specified destination index.

The rule will replace the existing one at the given destination moving
down every other rules. If destination is greater or equal the number of rules,
the rule is moved to the end.

## Usage

````nohighlight
irccdctl rule-move source destination
````

## Example

````nohighlight
irccdctl rule-move 0 5
irccdctl rule-move 4 3
````

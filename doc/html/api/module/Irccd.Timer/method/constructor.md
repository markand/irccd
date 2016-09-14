---
method: constructor
js: true
summary: "Create a new timer object."
synopsis: "Irccd.Timer(type, delay, callback) /* constructor */"
arguments:
  - "**type**: type of timer (`Irccd.Timer.Repeat` or `Irccd.Timer.Single`),"
  - "**delay**: the interval in milliseconds,"
  - "**callback**: the function to call."
---

## Example

````javascript
/* Execute an action every 1 second */
var t = new Irccd.Timer(Irccd.Timer.Repeat, 1000, function () {
    /* Do your action */
});

t.start();
````

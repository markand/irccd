---
module: Irccd
---

## Usage

Contains general irccd variables and functions.

## Constants

The following properties are defined:

  - **version**: (object)
    - **major**: (int) the major irccd version,
    - **minor**: (int) the minor irccd version,
    - **patch**: (int) the patch irccd version.

## Example

````javascript
var Logger = Irccd.Logger;

function onLoad()
	Logger.info("Major: " + Irccd.version.major);
	Logger.info("Minor: " + Irccd.version.minor);
	Logger.info("Patch: " + Irccd.version.patch);
end
````

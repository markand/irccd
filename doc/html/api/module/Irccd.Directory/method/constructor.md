---
method: constructor
summary: "Open a directory."
synopsis: "Irccd.Directory(path, flags) /* constructor */"
arguments:
  - "path, the path to the directory,"
  - "flags, the OR'ed flags: Irccd.Directory.Dot, Irccd.Directory.DotDot (Optional, default: none)"
throws: "Any exception on error."
---

When constructed successfully, the object has the following properties:

  - **count**: (int) the number of entries,
  - **path**: (string) the path to the directory,
  - **entries**: (array) an array for each entry containing:
    - **name**: (string) the base file name,
    - **type**: (int) the type of file (`Irccd.Directory.Type*`).

## Example

````javascript
try {
  var d = new Irccd.Directory("/usr/share/games");

  for (var i = 0; i < d.count; ++i) {
    if (d.entries[i].type == Irccd.Directory.TypeFile) {
      // use d.entries[i].name which is a file.
    }
  }
} catch (e) {
  // Use the error
}
````





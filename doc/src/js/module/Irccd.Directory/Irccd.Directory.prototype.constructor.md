# Function Irccd.Directory (constructor)

Open a directory.

When constructed successfully, the object has the following properties:

- path: (string) the path to the directory,
- entries: (array) an array for each entry containing:
  - name: (string) the base file name,
  - type: (int) the type of file (`Irccd.Directory.Type*`).

# Synopsis

```javascript
Irccd.Directory(path, flags)
```

# Arguments

- **path**: the path to the directory,
- **flags**: the OR'ed flags: `Irccd.Directory.Dot`, `Irccd.Directory.DotDot`
  (Optional, default: none).

# Throws

Any exception on error.

# Example

```javascript
try {
	var d = new Irccd.Directory("/usr/share/games");

	for (var i = 0; i < d.count; ++i)
		if (d.entries[i].type == Irccd.Directory.TypeFile)
			// use d.entries[i].name which is a file.
} catch (e) {
	// Use the error
}
```

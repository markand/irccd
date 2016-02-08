---
module: Irccd.File
summary: "File opening and writing."
---

## Usage

This class and functions are available for opening and writing files on the disk.

For convenience, some functions are available as free-functions and some as object methods.

<div class="alert alert-warning" role="alert">
**Warning**: the stat function and method are optional and may not be available on your system.
</div>

## Constants

The following properties are defined:

- **SeekCur**: (int) seek from the current file position,
- **SeekEnd**: (int) seek from end of the file,
- **SeekSet**: (int) seek from beginning of the file.

## Functions

- [basename](function/basename.html)
- [dirname](function/dirname.html)
- [exists](function/exists.html)
- [remove](function/remove.html)
- [stat](function/stat.html) (Optional)

## Methods

- [(constructor)](method/constructor.html)
- [basename](method/basename.html)
- [close](method/close.html)
- [dirname](method/dirname.html)
- [read](method/read.html)
- [readline](method/readline.html)
- [remove](method/remove.html)
- [seek](method/seek.html)
- [stat](method/stat.html) (Optional)
- [tell](method/tell.html)
- [write](method/write.html)

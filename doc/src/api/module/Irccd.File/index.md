# Module Irccd.File

This class and functions are available for opening and writing files on the
disk.

For convenience, some functions are available as free-functions and some as
object methods.

# Remarks

Warning: the stat function and method are optional and may not be available on
         your system.

# Constants

The following properties are defined:

  - **SeekCur**: (int) seek from the current file position,
  - **SeekEnd**: (int) seek from end of the file,
  - **SeekSet**: (int) seek from beginning of the file.

# Functions

  - [basename](Irccd.File.basename.html)
  - [dirname](Irccd.File.dirname.html)
  - [exists](Irccd.File.exists.html)
  - [remove](Irccd.File.remove.html)
  - [stat](Irccd.File.stat.html) (Optional)

# Methods

  - [(constructor)](Irccd.File.prototype.constructor.html)
  - [basename](Irccd.File.prototype.basename.html)
  - [close](Irccd.File.prototype.close.html)
  - [dirname](Irccd.File.prototype.dirname.html)
  - [lines](Irccd.File.prototype.lines.html)
  - [read](Irccd.File.prototype.read.html)
  - [readline](Irccd.File.prototype.readline.html)
  - [remove](Irccd.File.prototype.remove.html)
  - [seek](Irccd.File.prototype.seek.html)
  - [stat](Irccd.File.prototype.stat.html) (Optional)
  - [tell](Irccd.File.prototype.tell.html)
  - [write](Irccd.File.prototype.write.html)

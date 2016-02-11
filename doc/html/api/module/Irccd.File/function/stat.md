---
function: stat
summary: "Get file information at the specified path."
synopsis: "info = Irccd.File.stat(path)"
arguments:
  - "path, the path to the file."
returns: "The stat information."
throws: "Any exception on error."
---

The returned object may have the following properties if they are available on your system:

  - **atime**: (int) the last access time,
  - **blksize**: (int) the block size,
  - **blocks**: (int) the number of blocks,
  - **ctime**: (int) the creation time,
  - **dev**: (int) the device,
  - **gid**: (int) the group,
  - **ino**: (int) the inode,
  - **mode**: (int) the mode,
  - **mtime**: (int) the modification time,
  - **nlink**: (int) the number of hard links,
  - **rdev**: (int),
  - **size**: (int) the file size,
  - **uid**: (int) the user.

## Remarks

<div class="alert alert-warning" role="alert">
**Warning**: this function is optional and may not be available on your system.
</div>

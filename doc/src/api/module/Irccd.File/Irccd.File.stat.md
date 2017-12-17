# Function Irccd.File.stat

Get file information at the specified path.

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

Warning: this function is optional and may not be available on your system.

# Synopsis

```javascript
info = Irccd.File.stat(path)
```

# Arguments

  - **path**: the path to the file.

# Returns

The stat information.

# Throws

An [Irccd.SystemError](@baseurl@api/module/Irccd/index.html#types) on failures.

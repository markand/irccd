---
header: Building from sources
guide: yes
---

You should use the irccd version provided by your package manger if possible. If
irccd is not available, you can build it from sources.

# Requirements

To build from sources, you need the following installed on your system:

  - [CMake](http://www.cmake.org),
  - [OpenSSL](https://www.openssl.org) (Optional) for connecting with SSL,
  - [Pandoc](http://pandoc.org) (Optional) for building the documentation,
  - At least **GCC 5.1** or **clang 3.4**

<div class="alert alert-warning" role="alert">
**Warning**: don't even try to compile with GCC 4.x, it will not work due to missing C++14 features.
</div>

# Running the build

When you're ready, extract the **irccd-x.y.z.tar.gz** where **x.y.z** is the current version. Go to that directory,
then type the following commands:

````nohighlight
$ mkdir _build_
$ cd _build_
$ cmake ..
$ make
$ sudo make install
````

This is the quick way of compiling and installing. It's also possible to set some options to customize the build.

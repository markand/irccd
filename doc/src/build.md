% building from sources
% David Demelier
% 2017-12-08

You should use the irccd version provided by your package manger if possible. If
irccd is not available, you can build it from sources.

# Requirements

To build from sources, you need the following installed on your system:

  - [CMake](http://www.cmake.org),
  - [OpenSSL](https://www.openssl.org) (Optional) for connecting with SSL,
  - [Pandoc](http://pandoc.org) (Optional) for building the documentation,
  - At least **GCC 5.1** or **clang 3.4**

**Warning**: don't even try to compile with GCC 4.x, it will not work due to
             missing C++14 features.

# Running the build

When you're ready, extract the **irccd-x.y.z.tar.gz** where **x.y.z** is the
current version. Go to that directory, then type the following commands:

    $ mkdir _build_
    $ cd _build_
    $ cmake ..
    $ make
    $ sudo make install

This is the quick way of compiling and installing. It's also possible to set
some options to customize the build. See below.

# Customizing the build

You can configure some features when compiling irccd.

## Disabling JavaScript

You can disable JavaScript support.

    $ cmake .. -DIRCCD_WITH_JS=Off

## Disabling SSL

You can disable OpenSSL support, it is automatically unset if OpenSSL is not
found.

**Warning**: this is not recommended.

    $ cmake .. -DIRCCD_WITH_SSL=Off

## Disabling libedit

If for some reasons, you don't want auto-completion in `irccd-test`, you can
disable it.

    $ cmake .. -DIRCCD_WITH_LIBEDIT=Off

## Disabling all documentation

You can disable all the documentation.

    $ cmake .. -DIRCCD_WITH_DOCS=Off

See below to disable only specific parts of the documentation.

## Disabling HTML documentation

By default if Pandoc is available, the HTML documentation is built, you can
disable it.

    $ cmake .. -DWITH_HTML=Off

## Disabling man pages

You can disable installation of manuals.

    $ cmake .. -DIRCCD_WITH_MAN=Off

## Installation path

Sometimes, you may need to install irccd over other place, for that, you can
specify the prefix where to install files.

On Unix systems, it's usually **/usr/local** and **C:/Program Files/Irccd** on
Windows.

To change this, use the following:

    $ cmake .. -DCMAKE_INSTALL_PREFIX=/opt/some/directory

## Manual pages path

By default, irccd use **${CMAKE_INSTALL_PREFIX}/share/man** for manual pages.

For example, on FreeBSD the typical use would be:

    $ cmake .. -DIRCCD_WITH_MANDIR=/usr/local/man

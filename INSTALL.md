IRC Client Daemon INSTALL
=========================

This guide will help you to install irccd on your computer.

Requirements
------------

- GCC 7.0 or Clang 6.0,
- [Boost](http://boost.org),
- [CMake](http://www.cmake.org).

Optional:

- [OpenSSL](http://openssl.org), Used for SSL connections to IRC servers,
  recommended,
- [UriParser](), Required for the links plugin,
- [Doxygen](http://www.stack.nl/~dimitri/doxygen), For the documentation about
  irccd internals.

Basic installation
------------------

This is the quick way to install irccd.

    tar xvzf irccd-x.y.z-tar.xz
    cd irccd-x.y.z
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make
    sudo make install

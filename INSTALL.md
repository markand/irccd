IRC Client Daemon INSTALL
=========================

This guide will help you to install irccd on your computer. For a better guide
with more details see:

http://projects.malikania.fr/irccd/guide.html

Requirements
------------

  - GCC 5.1 or Clang 3.4,
  - [CMake](http://www.cmake.org). Used to build irccd.

Optional:

  - [OpenSSL](http://openssl.org), Used for SSL connections to IRC servers,
    recommended,
  - [Pandoc](http://pandoc.org), Used for documentation process,
  - [Doxygen](http://www.stack.nl/~dimitri/doxygen), For the documentation about
    irccd internals.

Basic installation
------------------

This is the quick way to install irccd.

    tar xvzf irccd-x.y.z-tar.xz
    cd irccd-x.y.z
    mkdir _build_
    cd _build_
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make
    sudo make install

IRC Client Daemon INSTALL
=========================

This guide will help you to install irccd

Requirements
------------

Build dependencies:

- C99 and few features from C11 (stdatomics, stdnoreturn).
- [Bison][] and [Flex][]: For configuration files.
- [CMake][]: Portable build system.

Optional runtime dependencies:

- [OpenSSL][]: Used for SSL connections to IRC servers (recommended).
- [CURL][]: Required for the links plugin.

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

[Bison]: https://www.gnu.org/software/bison
[CMake]: http://www.cmake.org
[CURL]: https://curl.se
[Flex]: https://github.com/westes/flex
[OpenSSL]: http://openssl.org

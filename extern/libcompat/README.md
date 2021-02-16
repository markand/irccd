libcompat
=========

This project contains several polyfill for various C extensions that are not
part of C standard or POSIX.

For example, it contains various functions such as:

- pledge: privileges separation from OpenBSD (no-op on other systems),
- reallocarray/recallocarray: safe alternative to realloc for arrays from
  OpenBSD,
- strlcpy/strlcat: safer alternatives to strncpy/strncat from OpenBSD,
- strsep: better alternative to strtok from 4.4BSD.

Requirements
------------

- C compiler,
- POSIX conformant system (ar, make, sh, rm, tr, touch).

Usage
-----

The easiest way to use libcompat is to copy the directory content into your
project and to adapt files you need.

Every routine (or group of routines) is implemented into an individual file
plus tested with a test file.

Example with strlcpy:

- libcompat/strlcpy.c
- libcompat/test-strlcpy.c

Then, the build process create the following output files, example with
strlcpy:

- libcompat/strlcpy.t: output of the compiler command, usually empty on
  success.
- libcompat/strlcpy.h: a header file containing `#define COMPAT_HAVE_STRLCPY`
  if the test succeed.
- libcompat/strlcpy.o: object file containing fallback implementation or
  nothing if the component is available in the host system.

Finally, when you want to use one of the extension, you'll have to import the
header file where it's supposed to be declared (unless this header file is not
portable) and you also must include libcompat/compat.h.

Example with strlcpy:

	#include <string.h>
	#include "libcompat/compat.h"

### Wrapper script

The convenient script libcompat/compile.sh will try co compile the test and
will either compile the polyfill or a dummy empty file instead. It should be
invoked with the canonical filename (without extension).

It also read `CC`, `CFLAGS` and `LDFLAGS` environment variable which are all
optional.

Example of use:

	./libcompat/compile.sh strlcpy
	CC="clang" CFLAGS="-O2" ./libcompat/compile.sh strlcat

### Makefile

The included Makefile can be used as boilerplate into your project and create a
libcompat.a archive ready to use. Simply adapt macros in the Makefile for your
own use.

Licenses
--------

Polyfill code are licensed per their authors and must be checked individually.
See headers in the libcompat directory.

Author
------

David Demelier <markand@malikania.fr>

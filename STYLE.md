IRC Client Daemon CODING STYLE
==============================

General rules
-------------

  - Never write two blank consecutives blank lines,
  - No jokes,
  - No easter eggs.

Style
-----

  - Always use 4 spaces as indentation,
  - Do not exceed 120 characters for lines of code,
  - Do not exceed 80 characters for comments.

### Braces

Braces follow the K&R style, they are never placed on their own lines except for
function definitions.

In addition to the K&R style, they are required everywhere even if a block
contains only one statement.

    if (condition) {
        apply();
        add();
    } else {
        ok();
    }

    if (condition) {
        validate();
    }

And a lambda has its braces on the same lines too:

    sort([&] (object&) {
        return true;
    });

### Naming

  - English names,
  - Member variables starts with `m_`,
  - No hungarian notation.

All functions, variables, class names are always camelCase. Only namespaces must
be all lowercase, short and concise. Please note that you should not create a
new namespace except irccd and anonymous ones.

    int m_variable;

    void myFunction()
    {
    }

### Files

  - Use `.cpp` and `.hpp` as file extensions,
  - Filenames are all lowercase.

### Comments

Avoid useless comments in source files. Comment complex things or why it is done
like this. However any public function in the .hpp **must** be documented as
doxygen without exception.

    /*
     * Multi line comments look like
     * this.
     */

    // Short comment

### Includes

The includes should always come in the following order.

  1. C++ headers
  2. C header
  3. Third party libraries
  4. Application headers in ""

    #include <cstring>
    #include <cerrno>

    #include <sys/stat.h>

    #include <libircclient.h>

    #include "foo.h"

**Note**: always use C++ headers for C equivalent, stdio.h -> cstdio, etc.

### Commit messages

Commit messages are written using the following syntax:

    Topic: short message less than 80 characters

    Optional additional description if needed.

Replace `Topic` with one of the following:

  - **CMake**: for the build system,
  - **Docs**: for the documentation,
  - **Irccd**: for the `irccd(1)` daemon,
  - **Irccdctl**: for the `irccdctl(1)` utility,
  - **Misc**: for miscellaneous files,
  - **Tests**: for the unit tests,
  - **Plugin xyz**: for a specific plugin (e.g. Plugin hangman:).

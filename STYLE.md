irccd CODING STYLE
==================

File content
============

- Use UTF-8 charset,
- Use Unix line endings,
- Never write two consecutives blank lines.

Indent
======

Use tabs to indent and spaces for alignment. Tabs are meant and designed for
indenting code and have the advantage of being configurable. On the other hand
to keep code clean, you must align content with spaces only *within* a line.

Note: we recommend 8 columns to avoid high number of indentations.

Example (show whitespace in your editor)

```cpp
class foo {
public:
	enum type {
		dummy_value,            // dummy comment
		other_value             // other comment
	};

	void long_function_name(very_long_type x1,
	                        very_long_type x2)
	{
		const map<string, string> m{
			{ "hostname",   "127.0.0.1"     },
			{ "port",       "6667"          }
		};
	}
};
```

As a rule of thumb, tabs must always be all length.

Example of incorrect usage:

```cpp
	{ "hostname",	"127.0.0.1"	},
	{ "port",	"6667"		}
```

This example will not align correctly if tabstops are not set to 8.

C
=

Style
-----

- Do not exceed 80 columns.

### Braces

Braces follow the K&R style, they are never placed on their own lines except for
function definitions.

Do not put braces for single line statements.

```c
if (condition) {
	apply();
	add();
} else
	ok();

if (condition)
	validate();

if (foo)
	state = long + conditional + that + requires + several + lines +
	        to + complete;
```

Functions require braces on their own lines.

```c
void
function()
{
}
```

Note: the type of a function is broken into its own line.

### Spaces

Each reserved keyword (e.g. `if`, `for`, `while`) requires a single space before
its argument.

Normal function calls do not require it.

```c
if (foo)
	destroy(sizeof (int));
```

### Pointers

Pointers are always next variable name.

```c
void
cleanup(struct owner *owner);
```

### Typedefs

Do not use typedef for non-opaque objects. It is allowed for function pointers.

```c
struct pack {
	int x;
	int y;
};

typedef void (*logger)(const char *line);
```

Note: do never add `_t` suffix to typedef's.

### Naming

- English names,
- No hungarian notation,

Almost everything is in `underscore_case` except macros and enumeration
constants.

```c
#if defined(FOO)
#	include <foo.hpp>
#endif

#define MAJOR 1
#define MINOR 0
#define PATCH 0

enum color {
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE
};

void
add_widget(const struct widget *widget_to_add);
```

### Header guards

Do not use `#pragma once`.

Header guards are usually named `PROJECT_COMPONENT_FILENAME_H`.

```c
#ifndef FOO_COMMON_UTIL_H
#define FOO_COMMON_UTIL_H

#endif /* !FOO_COMMON_UTIL_HPP */
```

### Enums

Enumerations constants are always defined in separate line to allow commenting
them as doxygen.

Note: enumeration constants are prefixed with its name.

```c
enum color {
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE
};
```

### Switch

In a switch case statement, you **must** not declare variables and not indent
cases.

```c
switch (variable) {
case foo:
	do_some_stuff();
	break;
default:
	break;
}
```

### Files

- Use `.c` and `.h` as file extensions,
- Filenames are all lowercase.

### Comments

Avoid useless comments in source files. Comment complex things or why it is done
like this. Do not use `//` style comments in C.

```c
/*
 * Multi line comments look like
 * this.
 */

/* Short comment. */
```

Use `#if 0` to comment blocks of code.

```c
#if 0
	broken_stuff();
#endif
```

### Includes

The includes should always come in the following order.

1. System headers (POSIX mostly)
2. C header
3. Third party libraries
4. Application headers in ""

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <libircclient.h>

#include "foo.h"
```

Programming
-----------

### C Standard

Use C11 standard without extensions.

### Assertions

Use the `assert` macro from the assert.h header file to verify programming
errors.

For example, you may use `assert` to verify that the developer access the data
between the bounds of an array:

```c
int
get(struct data *data, unsigned index)
{
	assert(index < data->length);

	return data->buffer[index];
}
```

The `assert` macro is not meant to check that a function succeeded, this code
must not be written that way:

```c
assert(listen(10));
```

### Return

The preferred style is to return early in case of errors. That makes the code
more linear and not highly indented.

This code is preferred:

```c
if (a_condition_is_not_valid)
	return false;
if (an_other_condition)
	return false;

start();
save();

return true;
```

Additional rules:

- Do never put parentheses between the returned value,
- Do not put a else branch after a return.

Shell
=====

Write POSIX shell only with no bash, zsh or any extension.

Style
-----

- Try to keep lines shorter than 80 columns

Functions
---------

Don't use `function` keyword, just keep the function name.

```sh
usage()
{
}
```

It's usually recommended to prefix functions names with the program name to
avoid collisions with global commands.

```sh
foo_clean()
{
}

foo_process()
{
}
```

Options
-------

For options, use `getopts` and prefer short options to long unless necessary.
Also set OPTERR=0 to avoid printing errors and do it yourself for UX
consistency.

```sh
OPTERR=0
while getopts "v" arg; do
	case $arg in
	v)
		verbose=1
		;;
	esac
done
```

Naming
------

Use `UPPERCASE` variables for global variables and `lowercase` for temporary or
local variables.

CMake
=====

Style
-----

- Try to keep lines shorter than 80 columns

### Spaces

Each programming keyword (e.g. `if`, `foreach`, `while`) requires a single space
before its argument, otherwise write opening parenthese directly after.

```cmake
foreach (c ${COMPONENTS})
	string(TOUPPER ${c} CMP)

	if (${WITH_${CMP}})
		add_executable(${c} ${c}.cpp)
	endif ()
endforeach ()
```

### Line breaks

When CMake lines goes too long, you should indent arguments at the same level,
it's also common to see named argument values indented even more.

```cmake
set(
	FILES
	${myapp_SOURCE_DIR}/main.cpp
	${myapp_SOURCE_DIR}/foo.cpp
	${myapp_SOURCE_DIR}/bar.cpp
)

command_with_lot_of_arguments(
	TARGET foo
	INSTALL On
	SOURCES
		${myapp_SOURCE_DIR}/main.cpp
		${myapp_SOURCE_DIR}/foo.cpp
		${myapp_SOURCE_DIR}/bar.cpp
	COMMENT "Some comment"
```

Modern CMake
------------

CMake evolves over time, if you have read very old articles there is a chance
that what you have read is actually deprecated and replaced by other features.
The following list is a short summary of modern CMake features that you must
use.

### Imported targets

When they are available, use imported targets rather than plain variables. They
offer complete dependency tracking with options and include directories as well.

```cmake
find_package(Boost COMPONENTS system)
target_link_libraries(main Boost::system)
```

### Generator expressions

Use generator expressions when it make sense. For example you should use them
for variables that are not used at generation time (e.g CMAKE\_BUILD\_TYPE).

```cmake
target_include_directories(
	myapp
		$<BUILD_INTERFACE:${myapp_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
```

Warning: do never test against `CMAKE_BUILD_TYPE` in any CMakeLists.txt, IDEs
         like Visual Studio will mismatch what you'll put in the conditions.

### Avoid global scoping

The following commands must be avoided as much as possible:

- `link_directories`,
- `link_libraries`,
- `include_directories`,
- `add_definitions`.

They pollute the global namespace, all targets defined after these commands will
be built against those settings. Instead, you should use the per-targets
commands.

```cmake
target_include_directories(
	mylib
	PUBLIC
		$<BUILD_INTERFACE:${mylib_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
target_link_libraries(mylib foo)
```

### Defining sources

You MUST never use any kind of `file(GLOB)` commands to list sources for an
executable. CMake is designed to be re-called by itself only when required,
having such a construct will not let CMake be able to detect if you have
added/removed files in your source directory. Instead, you MUST always specify
all source by hands.

```cmake
set(
	FILES
	${myapp_SOURCE_DIR}/main.cpp
	${myapp_SOURCE_DIR}/a.cpp
	${myapp_SOURCE_DIR}/b.cpp
)

add_executable(myapp ${FILES})
```

Markdown
========

Headers
-------

For 1st and 2nd level headers, use `===` and `---` delimiters and underline the
whole title. Otherwise use `###`.

```markdown
Top level title
===============

Sub title
---------

### Header 3

#### Header 4

##### Header 5

###### Header 6
```

Lists
-----

Use hyphens for unordered lists for consistency, do not indent top level lists,
then indent by two spaces each level

```markdown
- unordered list 1
- unordered list 2
  - sub unordered item

1. unordered list 1
2. unordered list 2
  2.1. sub unordered item
```

Code blocks
-----------

You can use three backticks and the language specifier or just indent a block by
for leading tabs if you don't need syntax.

	```cpp
	std::cout << "hello world" << std::endl;
	```

And without syntax:

```markdown
	This is simple code block.
```

Tables
------

Tables are supported and formatted as following:

```markdown
| header 1 | header 2 |
|----------|----------|
| item 1   | item 2   |
```

Alerts
------

It's possible to prefix a paragraph by one of the following topic, it renders a
different block depending on the output format:

- Note:
- Warning:
- Danger:

Then, if the paragraph is too long, indent the text correctly.

```markdown
Note: this is an information block that is too long to fit between the limits so
      it is split and indented.
```

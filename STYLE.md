IRC Client Daemon CODING STYLE
==============================

File content
============

- Use UTF-8 charset,
- Use Unix line endings,
- Never write two blank consecutives blank lines.

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

C++
===

Style
-----

- Do not exceed 120 columns for lines of code,
- Do not exceed 80 columns for comments,

### Braces

Braces follow the K&R style, they are never placed on their own lines except for
function definitions.

Do not put braces for single line statements.

```cpp
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

```cpp
void function()
{
}
```

And a lambda has its braces on the same lines too:

```cpp
sort([&] (auto&) {
	return true;
});
```

### Spaces

Each reserved keyword (e.g. `if`, `for`, `while`) requires a single space before
its argument.

Normal function calls do not require it.

```cpp
if (foo)
	destroy(sizeof (int));
```

### References and pointers

References and pointers are always next to the type name and not the variable.

```cpp
auto get(const std::string& name) -> T&;

int* p = &x;
```

### Trailing return syntax

We use trailing return syntax everywhere, it has the following benefits:

- Inner types don't need to be prefixed by class name,
- Functions are kept aligned correctly, focusing on the function name.

```cpp
auto func() -> std::string;
```

### Naming

- English names,
- Member variables have trailing underscore (e.g foo\_bar\_),
- No hungarian notation.

Everything is in `underscore_case` except template parameters and macros.

```cpp
#if defined(FOO)
#	include <foo.hpp>
#endif

namespace baz {

class object {
private:
	std::string name_;

public:
	auto name() const noexcept -> const std::string&;
};

template <typename Archive>
void open(const Archive& ar)
{
	bool is_valid = false;
}

} // !baz
```

### Header guards

Do not use `#pragma once`.

Header guards are usually named `PROJECT_COMPONENT_FILENAME_HPP`.

```cpp
#ifndef FOO_COMMON_UTIL_HPP
#define FOO_COMMON_UTIL_HPP

#endif // !FOO_COMMON_UTIL_HPP
```

### Enums

Enumerations constants are always defined in separate line to allow commenting
them as doxygen.

Enum class are encouraged.

```cpp
enum class color {
	blue,
	red,
	green
};
```

### Switch

In a switch case statement, you **must** not declare variables and not indent
cases.

```cpp
switch (variable) {
case foo:
	do_some_stuff();
	break;
default:
	break;
}
```

### Files

- Use `.cpp` and `.hpp` as file extensions,
- Filenames are all lowercase.

### Comments

Avoid useless comments in source files. Comment complex things or why it is done
like this. However any public function in the .hpp **must** be documented as
doxygen without exception.

```cpp
/*
 * Multi line comments look like
 * this.
 */

// Short comment
```

Use `#if 0` to comment blocks of code.

```cpp
#if 0
	broken_stuff();
#endif
```

### Includes

The includes should always come in the following order.

1. C++ headers
2. C header
3. Third party libraries
4. Application headers in ""

```cpp
#include <cstring>
#include <cerrno>

#include <sys/stat.h>

#include <libircclient.h>

#include "foo.h"
```

Note: always use C++ headers for C equivalent, stdio.h -> cstdio, etc.

Programming
-----------

### C language

Do not use old C stuff like `void*`, `srand/rand`, `printf` or anything that
can be rewritten in modern C++.

### RTTI

Usage of `dynamic_cast` and `typeid` are completely disallowed in any shape of
form.

### Arguments

It is recommended to pass parameters by value or const reference. Usage of
non-const reference as output parameter is **discouraged** and should be avoided
in many case because it does not allow chaining of expressions like:

```cpp
std::cout << reverse(upper(clean("  hello world!  "))) << std::endl;
```

If your function is designed to return a modified value passed as argument, it
is better to take it by value and modify it directly.

```cpp
auto clean(std::string input) -> std::string
{
	if (!input.empty() && input.back() == '\r')
		input.pop_back();

	return input;
}
```

Never pass primitive types as const value.

### Assertions

Use the `assert` macro from the cassert header file to verify programming
errors.

For example, you may use `assert` to verify that the developer access the data
between the bounds of an array:

```cpp
auto operator[](unsigned index) -> T&
{
	assert(index < length_);

	return data_[index];
}
```

The `assert` macro is not meant to check that a function succeeded, this code
must not be written that way:

```cpp
assert(listen(10));
```

### Exceptions

You must use exceptions to indicate an error that was unexpected such as:

- Failing to open a file,
- I/O unexpected errors,
- Parsing errors,
- User errors.

You may use the C++ standard exceptions defined in the stdexcept header but if
you need to carry more data within your exception, you should derive from
`std::exception`.

### Error code

You should not use error codes to indicate errors, instead use exceptions.
Error codes are allowed in Boost.Asio though.

### Free functions

Basic utility functions should be defined in a namespace as a free function not
as a static member function, we're doing C++ not Java.

Example:

```cpp
namespace util {

auto clean(std::string input) -> std::string;

} // !util
```

### Variables initialization

Use parentheses to initialize non primitive types:

```cpp
throw std::runtime_error("foo");

my_class obj("bar");
```

Use brace initialization when you want to use an initializer list, type
elision:

```cpp
std::vector<int> v{1, 2, 3};

foo({1, 2});					// type deduced

return { "true", false };	   // std::pair returned
```

Use the assignment for primitive types:

```cpp
int x = 123;
bool is_valid = true;
```

### Classes

Classes are usually defined in the following order:

1. Public inner types (enums, classes),
2. Protected/private members and functions
3. Public static functions.
3. Public member functions
4. Public virtual functions.

```cpp
class foo {
public:
	enum class type {
		a,
		b
	};

private:
	int member_{0};

public:
	void some_function();
};
```

### Structs

Use structs for objects that only need to store public data and have no
invariants. For example PODs and traits match this criteria:

```cpp
struct point {
	int x{0};
	int y{0};
};

template <>
struct info_traits<point> {
	template <typename Archive>
	static void serialize(Archive& ar, const point& point)
	{
		ar.write(point.x);
		ar.write(point.y);
	}
};
```

### Return

The preferred style is to return early in case of errors. That makes the code
more linear and not highly indented.

This code is preferred:

```cpp
if (a_condition_is_not_valid)
	return nullptr;
if (an_other_condition)
	return nullptr;

auto x = std::make_shared<object>();

x->start();
x->save();

return x;
```

Additional rules:

- Do never put parentheses between the returned value,
- Do not put a else branch after a return.

### Auto

We encorage usage of `auto`, it reduces code maintainance as you don't need to
change your code when your rename types.

```cpp
auto it = std::find_if(v.begin(), v.end(), [&] (const auto& obj) {
	return obj.key() == "foo";
});

for (const auto& pair : a_map)
	std::cout << pair.first << " = " << pair.second << std::endl;
```

But do not use `auto` to write code like in python, this is not acceptable:

```cpp
auto o = my_object("foo");
```

### String views

Use `std::string_view` each time you need a string that you will not own, this
includes: temporary arguments, return values, compile time constants.

```cpp
const std::string_view version("1.0");

void load(std::string_view id, std::string_view path)
{
	std::cout << "loading: " << id << " from path: " << path << std::endl;
}
```

### Optional values

Use `std::optional` to indicate a null value considered as valid. For example,
searching a value that may not exist.

```cpp
auto find(std::string_view id) -> std::optional<int>
{
	if (auto it = foo.find(id); it != foo.end())
		return it->second;

	return std::nullopt;
}
```

### Avoid definitions in headers

Try to avoid as much as possible function definition in header file. It slow
down compilation because the compiler has to parse the syntax over and over.
It's even worse as you may need to recompile a lot of files when you change a
header rather than a source file.

CMake
=====

Style
-----

- Try to keep line shorter than 80 columns

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
for leading spaces if you don't need syntax.

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

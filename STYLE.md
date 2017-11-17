PROJECT NAME CODING STYLE
=========================

C++
===

Style
-----

  - Always use 4 spaces as indentation,
  - Use UTF-8 charset,
  - Use Unix line endings,
  - Do not exceed 120 characters for lines of code,
  - Do not exceed 80 characters for comments,
  - Never write two blank consecutives blank lines,
  - Do not use bad words.

### Braces

Braces follow the K&R style, they are never placed on their own lines except for
function definitions.

Do not put braces for single line statements except for clarity.

    if (condition) {
        apply();
        add();
    } else
        ok();

    if (condition)
        validate();

    if (foo) {
        state = long + conditional + that + requires + several + lines +
            to + complete;
    }

Functions require braces on their own lines.

    void function()
    {
    }

And a lambda has its braces on the same lines too:

    sort([&] (auto&) {
        return true;
    });

### Spaces

Each reserved keyword (e.g. `if`, `for`, `while`) requires a single space before
its argument.

Normal function calls do not require it.

    if (foo)
        destroy(sizeof (int));

### References and pointers

References and pointers are always next to the type name and not the variable.

    T& get(const std::string& name);

    int* p = &x;

### Naming

  - English names,
  - Member variables have trailing underscore (e.g foo\_bar\_),
  - No hungarian notation.

Everything is in `underscore_case` except template parameters and macros.

    #if defined(FOO)
    #   include <foo.hpp>
    #endif

    namespace baz {

    class object {
    private:
        std::string name_;

    public:
        inline const std::string& name() const noexcept
        {
            return name_;
        }
    };

    template <typename Archive>
    void open(const Archive& ar)
    {
        bool is_valid = false;
    }

    } // !baz

### Header guards

Do not use `#pragma once`.

Header guards are usually named **PROJECT_COMPONENT_FILENAME_HPP**.

    #ifndef FOO_COMMON_UTIL_HPP
    #define FOO_COMMON_UTIL_HPP

    #endif // !FOO_COMMON_UTIL_HPP

### Enums

Enumerations constants are always defined in separate line to allow commenting
them as doxygen.

Enum class are encouraged.

    enum class color {
        blue,
        red,
        green
    };

### Switch

In a switch case statement, you **must** not declare variables and not indent
cases.

    switch (variable) {
    case foo:
        do_some_stuff();
        break;
    default:
        break;
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

Use `#if 0` to comment blocks of code.

#if 0
    broken_stuff();
#endif

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
  - **Misc**: for miscellaneous files,
  - **Tests**: for the unit tests.

Programming
-----------

### C language

Do not use old C stuff like `void *`, `srand/rand`, `printf` or anything that
can be rewritten in modern C++.

### RTTI

Usage of `dynamic_cast` and `typeid` are completely disallowed in any shape of
form.

### Arguments

It is recommended to pass parameters by value or const reference. Usage of
non-const reference as output parameter is **discouraged** and should be avoided
in many case because it does not allow chaining of expressions like:

    std::cout << reverse(upper(clean("  hello world!  "))) << std::endl;

If your function is designed to return a modified value passed as argument, it
is better to take it by value and modify it directly.

    std::string clean(std::string input)
    {
        if (!input.empty() && input.back() == '\r')
            input.pop_back();

        return input;
    }

Never pass primitive types as const value.

### Assertions

Use the `assert` macro from the cassert header file to verify programming
errors.

For example, you may use `assert` to verify that the developer access the data
between the bounds of an array:

    T& operator[](unsigned index)
    {
        assert(index < length_);

        return data_[index];
    }

The `assert` macro is not meant to check that a function succeeded, this code
must not be written that way:

    assert(listen(10));

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

    namespace util {

    std::string clean(std::string input);

    } // !util

### Variables initialization

Use parentheses to initialize non primitive types:

    throw std::runtime_error("foo");

    my_class obj("bar");

Use brace initialization when you want to use an initializer list, type
elision:

    std::vector<int> v{1, 2, 3};

    foo({1, 2});                    // type deduced

    return { "true", false };       // std::pair returned

Use the assignment for primitive types:

    int x = 123;
    bool is_valid = true;

### Classes

Classes are usually defined in the following order:

  1. Public inner types (enums, classes),
  2. Protected/private members
  3. Public functions

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

### Structs

Do not use C structs unless you have very good reason to do so. If you want to
pack some data, just use `class` and make all fields public.

    class point {
    public:
        int x{0};
        int y{0};
    };

### Return

The preferred style is to return early in case of errors. That makes the code
more linear and not highly indented.

This code is preferred:

    if (a_condition_is_not_valid)
        return nullptr;
    if (an_other_condition)
        return nullptr;

    auto x = std::make_shared<object>();

    x->start();
    x->save();

    return x;

Additional rules:

  - Do never put parentheses between the returned value,
  - Do not put a else branch after a return.

### Auto

We encorage usage of `auto`, it reduces code maintainance as you don't need to
change your code when your rename types.

````cpp
auto it = std::find_if(v.begin(), v.end(), [&] (const auto& obj) {
    return obj.key() == "foo";
});

for (const auto& pair : a_map)
    std::cout << pair.first << " = " << pair.second << std::endl;
````

But do not use `auto` to write code like in python, this is not acceptable:

````cpp
    auto o = my_object("foo");
````

/*
 * dynlib.hpp -- portable shared library loader
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef IRCCD_DYNLIB_HPP
#define IRCCD_DYNLIB_HPP

/**
 * \file dynlib.hpp
 * \brief Portable shared library loader.
 * \author David Demelier <markand@malikania.fr>
 */

/**
 * \page Dynlib Dynlib
 * \brief Portable shared library loader.
 *
 * The dynlib module let you open shared libraries dynamically at runtime.
 *
 * ## Operating system support
 *
 * | System  | Support | Remarks            |
 * |---------|---------|--------------------|
 * | Apple   | Ok      |                    |
 * | FreeBSD | Ok      |                    |
 * | Linux   | Ok      | Needs -ldl library |
 * | Windows | Ok      |                    |
 *
 * ## How to export symbols
 *
 * When you want to dynamically load symbols from your shared library, make sure they are in a `extern "C"` block, if
 * not they will be [mangled][name-mangling].
 *
 * Note, this does not mean that you can't write C++ code, it just mean that you can't use namespaces and function
 * overloading.
 *
 * Example of **plugin.cpp**:
 *
 * ````cpp
 * #include <iostream>
 *
 * #include "dynlib.hpp"
 *
 * extern "C" {
 *
 * DYNLIB_EXPORT void plugin_load()
 * {
 *   std::cout << "Loading plugin" << std::endl;
 * }
 *
 * DYNLIB_EXPORT void plugin_unload()
 * {
 *   std::cout << "Unloading plugin" << std::endl;
 * }
 *
 * }
 * ````
 *
 * The \ref DYNLIB_EXPORT macro is necessary on some platforms to be sure that symbol will be visible. Make sure you always
 * add it before any function.
 *
 * To compile, see your compiler documentation or build system. For gcc you can use the following:
 *
 * ````
 * gcc -std=c++14 -shared plugin.cpp -o plugin.so
 * ````
 *
 * ## How to load the library
 *
 * The dynlib module will search for the library in various places, thus you can use relative paths names but be sure
 * that the library can be found. Otherwise, just use an absolute path to the file.
 *
 * ````cpp
 * #include <iostream>
 *
 * #include "dynlib.hpp"
 *
 * int main()
 * {
 *   try {
 *     Dynlib dso("./plugin" DYNLIB_SUFFIX);
 *   } catch (const std::exception &ex) {
 *     std::cerr << ex.what() << std::endl;
 *   }
 *
 *   return 0;
 * }
 * ````
 *
 * ## How to load symbol
 *
 * The last part is symbol loading, you muse use raw C function pointer and the Dynlib::sym function.
 *
 * ````cpp
 * #include <iostream>
 *
 * #include "dynlib.hpp"
 *
 * using PluginLoad = void (*)();
 * using PluginUnload = void (*)();
 *
 * int main()
 * {
 *    try {
 *        Dynlib dso("./plugin" DYNLIB_SUFFIX);
 *
 *        dso.sym<PluginLoad>("plugin_load")();
 *        dso.sym<PluginUnload>("plugin_unload")();
 *    } catch (const std::exception &ex) {
 *        std::cerr << ex.what() << std::endl;
 *    }
 *
 *    return 0;
 * }
 * ````
 *
 * [name-mangling]: https://en.wikipedia.org/wiki/Name_mangling
 */

#include <stdexcept>
#include <string>

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

/**
 * \brief Export the symbol.
 *
 * This is required on some platforms and you should put it before your function signature.
 *
 * \code{.cpp}
 * extern "C" {
 *
 * DYNLIB_EXPORT void my_function()
 * {
 * }
 *
 * }
 * \endcode
 */
#if defined(_WIN32)
#  define DYNLIB_EXPORT    __declspec(dllexport)
#else
#  define DYNLIB_EXPORT
#endif

/**
 * \brief Usual suffix for the library.
 *
 * This macro expands to the suffix convention for this platform.
 *
 * \code{.cpp}
 * Dynlib library("./myplugin" DYNLIB_SUFFIX);
 * \endcode
 *
 * \note Don't use the suffix expanded value shown in Doxygen as it may be wrong.
 */
#if defined(_WIN32)
#  define DYNLIB_SUFFIX ".dll"
#elif defined(__APPLE__)
#  define DYNLIB_SUFFIX ".dylib"
#else
#  define DYNLIB_SUFFIX ".so"
#endif

namespace irccd {

/**
 * \brief Load a dynamic module.
 *
 * This class is a portable wrapper to load shared libraries on supported systems.
 */
class Dynlib {
private:
#if defined(_WIN32)
    using Handle    = HMODULE;
    using Sym    = FARPROC;
#else
    using Handle    = void *;
    using Sym    = void *;
#endif

public:
    /**
     * \brief Policy for symbol resolution.
     */
    enum Policy {
        Immediately,        //!< load symbols immediately
        Lazy            //!< load symbols when needed
    };

private:
    Handle    m_handle;

    Dynlib(const Dynlib &) = delete;
    Dynlib &operator=(const Dynlib &) = delete;

    Dynlib(Dynlib &&) = delete;
    Dynlib &operator=(Dynlib &&) = delete;

#if defined(_WIN32)
    std::string error()
    {
        LPSTR error = nullptr;
        std::string errmsg;

        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&error, 0, NULL);

        if (error) {
            errmsg = std::string(error);
            LocalFree(error);
        }

        return errmsg;
    }
#endif

public:
    /**
     * Constructor to load a shared module.
     *
     * \param path the absolute path
     * \param policy the policy to load
     * \throw std::runtime_error on error
     */
    inline Dynlib(const std::string &path, Policy policy = Immediately);

    /**
     * Close the library automatically.
     */
    inline ~Dynlib();

    /**
     * Get a symbol from the library.
     *
     * On some platforms the symbol must be manually exported.
     *
     * \param name the symbol
     * \return the symbol
     * \throw std::runtime_error on error
     * \see DYNLIB_EXPORT
     */
    template <typename T>
    inline T sym(const std::string &name);
};

#if defined(_WIN32)

/*
 * Windows implementation
 * ------------------------------------------------------------------
 */

Dynlib::Dynlib(const std::string &path, Policy)
{
    m_handle = LoadLibraryA(path.c_str());

    if (m_handle == nullptr)
        throw std::runtime_error(error());
}

Dynlib::~Dynlib()
{
    FreeLibrary(m_handle);
    m_handle = nullptr;
}

template <typename T>
T Dynlib::sym(const std::string &name)
{
    Sym sym = GetProcAddress(m_handle, name.c_str());

    if (sym == nullptr)
        throw std::runtime_error(error());

    return reinterpret_cast<T>(sym);
}

#else

/*
 * Unix implementation
 * ------------------------------------------------------------------
 */

Dynlib::Dynlib(const std::string &path, Policy policy)
{
    m_handle = dlopen(path.c_str(), policy == Immediately ? RTLD_NOW : RTLD_LAZY);

    if (m_handle == nullptr)
        throw std::runtime_error(dlerror());
}

Dynlib::~Dynlib()
{
    dlclose(m_handle);
    m_handle = nullptr;
}

template <typename T>
T Dynlib::sym(const std::string &name)
{
    Sym sym = dlsym(m_handle, name.c_str());

    if (sym == nullptr)
        throw std::runtime_error(dlerror());

    return reinterpret_cast<T>(sym);
}

#endif

#endif // !IRCCD_DYNLIB_HPP

} // !irccd

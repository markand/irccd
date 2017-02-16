/*
 * fs.hpp -- filesystem operations
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

#ifndef IRCCD_FS_HPP
#define IRCCD_FS_HPP

/**
 * \file fs.hpp
 * \brief Filesystem operations made easy.
 */

/**
 * \cond FS_HIDDEN_SYMBOLS
 */

#if !defined(FS_HAVE_STAT)
#   if defined(_WIN32)
#       define FS_HAVE_STAT
#   elif defined(__linux__)
#       define FS_HAVE_STAT
#   elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#       define FS_HAVE_STAT
#   elif defined(__APPLE__)
#       define FS_HAVE_STAT
#   endif
#endif

/**
 * \endcond
 */

#if defined(FS_HAVE_STAT)
#   include <sys/stat.h>
#endif

#include <regex>
#include <string>
#include <vector>

#include "sysconfig.hpp"

namespace irccd {

/**
 * \brief Filesystem namespace.
 */
namespace fs {

/**
 * \enum Flags
 * \brief Flags for readdir.
 */
enum Flags {
    Dot     = (1 << 0),         //!< if set, also lists "."
    DotDot  = (1 << 1)          //!< if set, also lists ".."
};

/**
 * \brief Entry in the directory list.
 */
class Entry {
public:
    /**
     * \brief Describe the type of an entry
     */
    enum Type : char {
        Unknown,                //!< File type is unknown,
        File,                   //!< File is regular type,
        Dir,                    //!< File is directory,
        Link                    //!< File is link
    };

    std::string name;           //!< name of entry (base name)
    Type type{Unknown};         //!< type of file
};

/**
 * Check if two entries are identical.
 *
 * \param e1 the first entry
 * \param e2 the second entry
 * \return true if they are identical
 */
inline bool operator==(const Entry &e1, const Entry &e2) noexcept
{
    return e1.name == e2.name && e1.type == e2.type;
}

/**
 * Check if two entries are different.
 *
 * \param e1 the first entry
 * \param e2 the second entry
 * \return true if they are different
 */
inline bool operator!=(const Entry &e1, const Entry &e2) noexcept
{
    return !(e1 == e2);
}

/**
 * Get the separator for that system.
 *
 * \return \ on Windows and / otherwise
 */
inline char separator() noexcept
{
#if defined(_WIN32)
    return '\\';
#else
    return '/';
#endif
}

/**
 * Clean a path by removing any extra / or \ and add a trailing one.
 *
 * \param path the path
 * \return the updated path
 */
IRCCD_EXPORT std::string clean(std::string path);

/**
 * Get the base name from a path.
 *
 * Example, baseName("/etc/foo.conf") // foo.conf
 *
 * \param path the path
 * \return the base name
 */
IRCCD_EXPORT std::string baseName(std::string path);

/**
 * Get the parent directory from a path.
 *
 * Example, dirName("/etc/foo.conf") // /etc
 *
 * \param path the path
 * \return the parent directory
 */
IRCCD_EXPORT std::string dirName(std::string path);

#if defined(FS_HAVE_STAT)

/**
 * Get stat information.
 *
 * \param path the path
 * \return the stat information
 * \throw std::runtime_error on failure
 */
IRCCD_EXPORT struct stat stat(const std::string &path);

#endif // !HAVE_STAT

/**
 * Check if the file is readable.
 *
 * \param path the path
 * \return true if has read access
 */
IRCCD_EXPORT bool isReadable(const std::string &path) noexcept;

/**
 * Check if the file is writable.
 *
 * \param path the path
 * \return true if has write access
 */
IRCCD_EXPORT bool isWritable(const std::string &path) noexcept;

/**
 * Check if the file is a regular file.
 *
 * \param path the path
 * \return true if it is a file and false if not or not readable
 * \throw std::runtime_error if the operation is not supported
 */
IRCCD_EXPORT bool isFile(const std::string &path);

/**
 * Check if the file is a directory.
 *
 * \param path the path
 * \return true if it is a directory and false if not or not readable
 * \throw std::runtime_error if the operation is not supported
 */
IRCCD_EXPORT bool isDirectory(const std::string &path);

/**
 * Check if the file is a symbolic link.
 *
 * \param path the path
 * \return true if it is a symbolic link and false if not or not readable
 * \throw std::runtime_error if the operation is not supported
 */
IRCCD_EXPORT bool isSymlink(const std::string &path);

/**
 * Read a directory and return a list of entries (not recursive).
 *
 * \param path the directory path
 * \param flags the optional flags (see Flags)
 * \return the list of entries
 * \throw std::runtime_error on failure
 */
IRCCD_EXPORT std::vector<Entry> readdir(const std::string &path, int flags = 0);

/**
 * Search an item recursively.
 *
 * The predicate must have the following signature:
 *  void f(const std::string &base, const Entry &entry)
 *
 * Where:
 *   - base is the current parent directory in the tree
 *   - entry is the current entry
 *
 * \param base the base directory
 * \param predicate the predicate
 * \return the full path name to the file or empty string if never found
 * \throw std::runtime_error on read errors
 */
template <typename Predicate>
std::string findIf(const std::string &base, Predicate &&predicate)
{
    /*
     * Do not go deeply to the tree before testing all files in the current
     * directory for performances reasons, we iterate this directory to search
     * for the entry name and iterate again over all sub directories if not
     * found.
     */
    std::string path;
    std::vector<Entry> entries = readdir(base);

    for (const auto &entry : entries) {
        if (predicate(base, entry)) {
            path = base + separator() + entry.name;
            break;
        }
    }

    if (!path.empty())
        return path;

    for (const auto &entry : entries) {
        if (entry.type != Entry::Dir)
            continue;

        path = findIf(base + separator() + entry.name, std::forward<Predicate>(predicate));

        if (!path.empty())
            break;
    }

    return path;
}

/**
 * Find a file by name recursively.
 *
 * \param base the base directory
 * \param name the file name
 * \return the full path name to the file or empty string if never found
 * \throw std::runtime_error on read errors
 */
inline std::string find(const std::string &base, const std::string &name)
{
    return findIf(base, [&] (const auto &, const auto &entry) { return entry.name == name; });
}

/**
 * Overload by regular expression.
 *
 * \param base the base directory
 * \param regex the regular expression
 * \return the full path name to the file or empty string if never found
 * \throw std::runtime_error on read errors
 */
inline std::string find(const std::string &base, const std::regex &regex)
{
    return findIf(base, [&] (const auto &, const auto &entry) { return std::regex_match(entry.name, regex); });
}

/**
 * Get the current working directory.
 *
 * \return the current working directory
 * \throw std::runtime_error on failure
 */
IRCCD_EXPORT std::string cwd();

} // !fs

} // !irccd

#endif // !IRCCD_FS_HPP

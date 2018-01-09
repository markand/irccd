/*
 * fs_util.hpp -- filesystem utilities
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_COMMON_FS_UTIL_HPP
#define IRCCD_COMMON_FS_UTIL_HPP

/**
 * \file fs_util.hpp
 * \brief Filesystem utilities.
 */

#include <regex>
#include <string>

#include <boost/filesystem.hpp>

namespace irccd {

namespace fs_util {

/**
 * Get the base name from a path.
 *
 * Example, baseName("/etc/foo.conf") // foo.conf
 *
 * \param path the path
 * \return the base name
 */
inline std::string base_name(const std::string& path)
{
    return boost::filesystem::path(path).filename().string();
}

/**
 * Get the parent directory from a path.
 *
 * Example, dirName("/etc/foo.conf") // /etc
 *
 * \param path the path
 * \return the parent directory
 */
inline std::string dir_name(std::string path)
{
    return boost::filesystem::path(path).parent_path().string();
}

/**
 * Search an item recursively.
 *
 * The predicate must have the following signature:
 *  void f(const boost::filesystem::directory_entry& entry)
 *
 * Where:
 *   - base is the current parent directory in the tree
 *   - entry is the current entry
 *
 * \param base the base directory
 * \param predicate the predicate
 * \param recursive true to do recursive search
 * \return the full path name to the file or empty string if never found
 * \throw std::runtime_error on read errors
 */
template <typename Predicate>
std::string find_if(const std::string& base, bool recursive, Predicate&& predicate)
{
    auto find = [&] (auto it) -> std::string {
        for (const auto& entry : it)
            if (predicate(entry))
                return entry.path().string();

        return "";
    };

    if (recursive)
        return find(boost::filesystem::recursive_directory_iterator(base));

    return find(boost::filesystem::directory_iterator(base));
}

/**
 * Find a file by name recursively.
 *
 * \param base the base directory
 * \param name the file name
 * \param recursive true to do recursive search
 * \return the full path name to the file or empty string if never found
 * \throw std::runtime_error on read errors
 */
inline std::string find(const std::string& base, const std::string& name, bool recursive = false)
{
    return find_if(base, recursive, [&] (const auto& entry) {
        return entry.path().filename().string() == name;
    });
}

/**
 * Overload by regular expression.
 *
 * \param base the base directory
 * \param regex the regular expression
 * \param recursive true to do recursive search
 * \return the full path name to the file or empty string if never found
 * \throw std::runtime_error on read errors
 */
inline std::string find(const std::string& base, const std::regex& regex, bool recursive = false)
{
    return find_if(base, recursive, [&] (const auto& entry) {
        return std::regex_match(entry.path().filename().string(), regex);
    });
}

} // !fs_util

} // !irccd

#endif // !IRCCD_COMMON_FS_UTIL_HPP

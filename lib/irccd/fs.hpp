/*
 * fs.hpp -- filesystem operations
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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
 * @file fs.hpp
 * @brief Filesystem operations made easy.
 *
 * The following options can be set by the user:
 *
  * - **HAVE_ACCESS**: (bool) Set to true if unistd.h file and access(2) function are available.
 *
 * On Windows, you must link to shlwapi library.
 */

#include <sys/stat.h>

#include <regex>
#include <string>
#include <vector>

namespace irccd {

namespace fs {

/**
 * @enum Flags
 * @brief Flags for readdir.
 */
enum Flags {
	Dot	= (1 << 0),	//!< if set, also lists "."
	DotDot	= (1 << 1)	//!< if set, also lists ".."
};

/**
 * @class Entry
 * @brief Entry in the directory list.
 */
class Entry {
public:
	/**
	 * @brief Describe the type of an entry
	 */
	enum Type : char {
		Unknown,	//!< File type is unknown,
		File,		//!< File is regular type,
		Dir,		//!< File is directory,
		Link		//!< File is link
	};

	std::string name;	//! name of entry (base name)
	Type type{Unknown};	//! type of file
};

/**
 * Check if two entries are identical.
 *
 * @param e1 the first entry
 * @param e2 the second entry
 * @return true if they are identical
 */
bool operator==(const Entry &e1, const Entry &e2) noexcept;

/**
 * Check if two entries are different.
 *
 * @param e1 the first entry
 * @param e2 the second entry
 * @return true if they are different
 */
bool operator!=(const Entry &e1, const Entry &e2) noexcept;

/**
 * Get the separator for that system.
 *
 * @return \ on Windows and / otherwise
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
 * @param path the path
 * @return the updated path
 */
std::string clean(std::string path);

/**
 * Get the base name from a path.
 *
 * Example, baseName("/etc/foo.conf") // foo.conf
 *
 * @param path the path
 * @return the base name
 */
std::string baseName(std::string path);

/**
 * Get the parent directory from a path.
 *
 * Example, dirName("/etc/foo.conf") // /etc
 *
 * @param path the path
 * @return the parent directory
 */
std::string dirName(std::string path);

/**
 * Get stat information.
 *
 * @param path the path
 * @return the stat information
 * @throw std::runtime_error on failure
 */
struct stat stat(const std::string &path);

/**
 * Check if a file exists.
 *
 * If HAVE_ACCESS is defined, the function access is used, otherwise stat is used.
 *
 * @param path the path to check
 * @return true if the path exists
 */
bool exists(const std::string &path) noexcept;

/**
 * Check if the path is absolute.
 *
 * @param path the path
 * @return true if the path is absolute
 */
bool isAbsolute(const std::string &path) noexcept;

/**
 * Check if the path is relative.
 *
 * @param path the path
 * @return true if the path is absolute
 */
bool isRelative(const std::string &path) noexcept;

/**
 * Check if the file is readable.
 *
 * @param path the path
 * @return true if has read access
 */
bool isReadable(const std::string &path) noexcept;

/**
 * Check if the file is writable.
 *
 * @param path the path
 * @return true if has write access
 */
bool isWritable(const std::string &path) noexcept;

/**
 * Check if the file is a regular file.
 *
 * @param path the path
 * @return true if it is a file and false if not or not readable
 */
bool isFile(const std::string &path) noexcept;

/**
 * Check if the file is a directory.
 *
 * @param path the path
 * @return true if it is a directory and false if not or not readable
 */
bool isDirectory(const std::string &path) noexcept;

/**
 * Check if the file is a symbolic link.
 *
 * @param path the path
 * @return true if it is a symbolic link and false if not or not readable
 */
bool isSymlink(const std::string &path) noexcept;

/**
 * Read a directory and return a list of entries (not recursive).
 *
 * @param path the directory path
 * @param flags the optional flags (see Flags)
 * @return the list of entries
 * @throw std::runtime_error on failure
 */
std::vector<Entry> readdir(const std::string &path, int flags = 0);

/**
 * Create a directory recursively.
 *
 * @param path the path
 * @param mode the optional mode (not always supported)
 * @throw std::runtime_error on failure
 * @post all intermediate directories are created
 */
void mkdir(const std::string &path, int mode = 0700);

/**
 * Remove a directory recursively.
 *
 * If errors happens, they are silently discarded to remove as much as possible.
 *
 * @param path the path
 */
void rmdir(const std::string &path) noexcept;

/**
 * Search an item recursively.
 *
 * The predicate must have the following signature:
 *	void f(const std::string &base, const Entry &entry)
 *
 * Where:
 *   - base is the current parent directory in the tree
 *   - entry is the current entry
 *
 * @param base the base directory
 * @param predicate the predicate
 * @return the full path name to the file or empty string if never found
 * @throw std::runtime_error on read errors
 */
template <typename Predicate>
std::string findIf(const std::string &base, Predicate &&predicate)
{
	/*
	 * Do not go deeply to the tree before testing all files in the current directory for performances reasons, we iterate
	 * this directory to search for the entry name and iterate again over all sub directories if not found.
	 */
	std::string path;
	std::vector<Entry> entries = readdir(base);

	for (const auto &entry : entries) {
		if (predicate(base, entry)) {
			path = base + separator() + entry.name;
			break;
		}
	}

	if (path.empty()) {
		for (const auto &entry : entries) {
			if (entry.type == Entry::Dir) {
				path = findIf(base + separator() + entry.name, std::forward<Predicate>(predicate));

				if (!path.empty())
					break;
			}
		}
	}

	return path;
}

/**
 * Find a file by name recursively.
 *
 * @param base the base directory
 * @param name the file name
 * @return the full path name to the file or empty string if never found
 * @throw std::runtime_error on read errors
 */
inline std::string find(const std::string &base, const std::string &name)
{
	return findIf(base, [&] (const auto &, const auto &entry) { return entry.name == name; });
}

/**
 * Overload by regular expression.
 *
 * @param base the base directory
 * @param regex the regular expression
 * @return the full path name to the file or empty string if never found
 * @throw std::runtime_error on read errors
 */
inline std::string find(const std::string &base, const std::regex &regex)
{
	return findIf(base, [&] (const auto &, const auto &entry) { return std::regex_match(entry.name, regex); });
}

/**
 * Get the current working directory.
 *
 * @return the current working directory
 * @throw std::runtime_error on failure
 */
std::string cwd();

} // !fs

} // !irccd

#endif // !IRCCD_FS_HPP

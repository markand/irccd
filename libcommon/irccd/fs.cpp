/*
 * fs.cpp -- filesystem operations
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

#if defined(_WIN32)
#   if !defined(_CRT_SECURE_NO_WARNINGS)
#       define _CRT_SECURE_NO_WARNINGS
#   endif
#   if !defined(WIN32_LEAN_AND_MEAN)
#       define WIN32_LEAN_AND_MEAN
#   endif
#endif

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <stdexcept>

#if defined(_WIN32)
#   include <direct.h>
#   include <windows.h>
#   include <shlwapi.h>
#else
#   include <sys/types.h>
#   include <dirent.h>
#   include <unistd.h>
#   include <climits>
#endif

#include "fs.hpp"

namespace irccd {

namespace fs {

namespace {

/*
 * error.
 * ------------------------------------------------------------------
 *
 * Function to retrieve system error in Windows API.
 */
#if defined(_WIN32)

std::string error()
{
    LPSTR error = nullptr;
    std::string errmsg = "Unknown error";

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&error, 0, nullptr);

    if (error) {
        errmsg = std::string(error);
        LocalFree(error);
    }

    return errmsg;
}

#endif

/*
 * hasAccess.
 * ------------------------------------------------------------------
 *
 * Check if we have access to the file specified, mode is the same used as
 * std::fopen.
 */
bool hasAccess(const std::string &path, const std::string &mode)
{
    assert(mode.length() == 1);
    assert(mode[0] == 'r' || mode[0] == 'w');

    auto fp = std::fopen(path.c_str(), mode.c_str());

    if (fp == nullptr)
        return false;

    std::fclose(fp);

    return true;
}

/*
 * typeOf.
 * ------------------------------------------------------------------
 *
 * Get the type of the specified file.
 *
 * Use GetFileAttributesA on Windows and stat if available.
 *
 * Receives the object as predicate parameter and return true on success.
 */
#if defined(_WIN32)

template <typename Predicate>
bool typeOf(const std::string &path, Predicate &&predicate)
{
    DWORD result = GetFileAttributesA(path.c_str());

    if (result == INVALID_FILE_ATTRIBUTES)
        return false;

    return predicate(result);
}

#elif defined(FS_HAVE_STAT)

template <typename Predicate>
bool typeOf(const std::string &path, Predicate &&predicate) noexcept
{
    struct stat st;

    if (::stat(path.c_str(), &st) < 0)
        return false;

    return predicate(st);
}

#else

template <typename Predicate>
bool typeOf(const std::string &path, Predicate &&predicate) noexcept
{
    throw std::runtime_error(std::strerror(ENOSYS));
}

#endif

} // !namespace

/*
 * clean.
 * ------------------------------------------------------------------
 */
std::string clean(std::string input)
{
    if (input.empty())
        return input;

    // First, remove any duplicates.
    input.erase(std::unique(input.begin(), input.end(), [&] (char c1, char c2) {
        return c1 == c2 && (c1 == '/' || c1 == '\\');
    }), input.end());

    // Add a trailing / or \\.
    char c = input[input.length() - 1];

    if (c != '/' && c != '\\')
        input += separator();

    // Now converts all / to \\ for Windows and the opposite for Unix.
#if defined(_WIN32)
    std::replace(input.begin(), input.end(), '/', '\\');
#else
    std::replace(input.begin(), input.end(), '\\', '/');
#endif

    return input;
}

/*
 * baseName.
 * ------------------------------------------------------------------
 */
std::string baseName(std::string path)
{
    auto pos = path.find_last_of("\\/");

    if (pos != std::string::npos)
        path = path.substr(pos + 1);

    return path;
}

/*
 * dirName.
 * ------------------------------------------------------------------
 */
std::string dirName(std::string path)
{
    auto pos = path.find_last_of("\\/");

    if (pos == std::string::npos)
        path = ".";
    else
        path = path.substr(0, pos);

    return path;
}

/*
 * isAbsolute.
 * ------------------------------------------------------------------
 */
bool isAbsolute(const std::string &path) noexcept
{
#if defined(_WIN32)
    return !isRelative(path);
#else
    return path.size() > 0 && path[0] == '/';
#endif
}

/*
 * isRelative.
 * ------------------------------------------------------------------
 */
bool isRelative(const std::string &path) noexcept
{
#if defined(_WIN32)
    return PathIsRelativeA(path.c_str()) == 1;
#else
    return !isAbsolute(path);
#endif
}

/*
 * isReadable.
 * ------------------------------------------------------------------
 */
bool isReadable(const std::string &path) noexcept
{
    return hasAccess(path, "r");
}

/*
 * isWritable.
 * ------------------------------------------------------------------
 */
bool isWritable(const std::string &path) noexcept
{
    return hasAccess(path, "w");
}

/*
 * isFile.
 * ------------------------------------------------------------------
 */
bool isFile(const std::string &path)
{
    return typeOf(path, [] (const auto &object) {
#if defined(_WIN32)
        return (object & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE;
#elif defined(FS_HAVE_STAT)
        return S_ISREG(object.st_mode);
#endif
    });
}

/*
 * isDirectory.
 * ------------------------------------------------------------------
 */
bool isDirectory(const std::string &path)
{
    return typeOf(path, [] (const auto &object) {
#if defined(_WIN32)
        return (object & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
#elif defined(FS_HAVE_STAT)
        return S_ISDIR(object.st_mode);
#endif
    });
}

/*
 * isSymlink.
 * ------------------------------------------------------------------
 */
bool isSymlink(const std::string &path)
{
    return typeOf(path, [] (const auto &object) {
#if defined(_WIN32)
        return (object & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT;
#elif defined(FS_HAVE_STAT)
        return S_ISLNK(object.st_mode);
#endif
    });
}

/*
 * stat.
 * ------------------------------------------------------------------
 */
#if defined(FS_HAVE_STAT)

struct stat stat(const std::string &path)
{
    struct stat st;

    if (::stat(path.c_str(), &st) < 0)
        throw std::runtime_error(std::strerror(errno));

    return st;
}

#endif

/*
 * exists.
 * ------------------------------------------------------------------
 */
bool exists(const std::string &path) noexcept
{
#if defined(FS_HAVE_STAT)
    struct stat st;

    return ::stat(path.c_str(), &st) == 0;
#else
    return  hasAccess(path, "r");
#endif
}

/*
 * readdir.
 * ------------------------------------------------------------------
 */
std::vector<Entry> readdir(const std::string &path, int flags)
{
    std::vector<Entry> entries;

#if defined(_WIN32)
    std::ostringstream oss;
    HANDLE handle;
    WIN32_FIND_DATA fdata;

    oss << path << "\\*";
    handle = FindFirstFile(oss.str().c_str(), &fdata);

    if (handle == nullptr)
        throw std::runtime_error(error());

    do {
        Entry entry;

        entry.name = fdata.cFileName;

        if (entry.name == "." && !(flags & Dot))
            continue;
        if (entry.name == ".." && !(flags & DotDot))
            continue;

        switch (fdata.dwFileAttributes) {
        case FILE_ATTRIBUTE_DIRECTORY:
            entry.type = Entry::Dir;
            break;
        case FILE_ATTRIBUTE_NORMAL:
            entry.type = Entry::File;
            break;
        case FILE_ATTRIBUTE_REPARSE_POINT:
            entry.type = Entry::Link;
            break;
        default:
            break;
        }

        entries.push_back(std::move(entry));
    } while (FindNextFile(handle, &fdata) != 0);

    FindClose(handle);
#else
    DIR *dp;
    struct dirent *ent;

    if ((dp = opendir(path.c_str())) == nullptr)
        throw std::runtime_error(std::strerror(errno));

    while ((ent = readdir(dp)) != nullptr) {
        Entry entry;

        entry.name = ent->d_name;
        if (entry.name == "." && !(flags & Dot))
            continue;
        if (entry.name == ".." && !(flags & DotDot))
            continue;

        switch (ent->d_type) {
        case DT_DIR:
            entry.type = Entry::Dir;
            break;
        case DT_REG:
            entry.type = Entry::File;
            break;
        case DT_LNK:
            entry.type = Entry::Link;
            break;
        default:
            break;
        }

        entries.push_back(std::move(entry));
    }

    closedir(dp);
#endif

    return entries;
}

/*
 * mkdir.
 * ------------------------------------------------------------------
 */
void mkdir(const std::string &path, int mode)
{
    std::string::size_type next = 0;
    std::string part;

    for (;;) {
        next = path.find_first_of("\\/", next);
        part = path.substr(0, next);

        if (!part.empty()) {
#if defined(_WIN32)
            (void)mode;

            if (::_mkdir(part.c_str()) < 0 && errno != EEXIST)
                throw std::runtime_error(std::strerror(errno));
#else
            if (::mkdir(part.c_str(), mode) < 0 && errno != EEXIST)
                throw std::runtime_error(std::strerror(errno));
#endif
        }

        if (next++ == std::string::npos)
            break;
    }
}

/*
 * rmdir.
 * ------------------------------------------------------------------
 */
void rmdir(const std::string &base) noexcept
{
    try {
        for (const auto &entry : readdir(base)) {
            std::string path = base + separator() + entry.name;

            if (entry.type == Entry::Dir)
                rmdir(path);
            else
                ::remove(path.c_str());
        }
    } catch (...) {
        // Silently discard to remove as much as possible.
    }

#if defined(_WIN32)
    ::RemoveDirectoryA(base.c_str());
#else
    ::remove(base.c_str());
#endif
}

/*
 * cwd.
 * ------------------------------------------------------------------
 */
std::string cwd()
{
#if defined(_WIN32)
    char path[MAX_PATH];

    if (!::GetCurrentDirectoryA(sizeof (path), path))
        throw std::runtime_error("failed to get current working directory");

    return path;
#else
    char path[PATH_MAX];

    if (::getcwd(path, sizeof (path)) == nullptr)
        throw std::runtime_error{std::strerror(errno)};

    return path;
#endif
}

} // !irccd

} // !fs

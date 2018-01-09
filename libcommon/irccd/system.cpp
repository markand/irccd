/*
 * system.cpp -- platform dependent functions for system inspection
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

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>

#include "sysconfig.hpp"

#if defined(HAVE_SETPROGNAME)
#   include <cstdlib>
#endif

#if defined(IRCCD_SYSTEM_LINUX)
#   include <sys/sysinfo.h>

#   include <unistd.h>

#   include <cerrno>
#   include <climits>
#   include <cstring>
#elif defined(IRCCD_SYSTEM_FREEBSD)                                         \
    || defined(IRCCD_SYSTEM_DRAGONFLYBSD)                                   \
    || defined(IRCCD_SYSTEM_NETBSD)                                         \
    || defined(IRCCD_SYSTEM_OPENBSD)
#   if defined(IRCCD_SYSTEM_NETBSD)
#       include <sys/param.h>
#   else
#       include <sys/types.h>
#   endif

#   if defined(IRCCD_SYSTEM_OPENBSD)
#       include <unistd.h>
#   endif

#   include <sys/sysctl.h>

#   include <cerrno>
#   include <climits>
#   include <cstddef>
#   include <cstdlib>
#   include <cstring>
#elif defined(IRCCD_SYSTEM_MAC)
#   include <sys/sysctl.h>
#   include <cerrno>
#   include <cstring>
#   include <libproc.h>
#   include <unistd.h>
#elif defined(IRCCD_SYSTEM_WINDOWS)
#   include <sys/types.h>
#   include <sys/timeb.h>
#   include <windows.h>
#   include <shlobj.h>
#endif

#if !defined(IRCCD_SYSTEM_WINDOWS)
#   include <sys/utsname.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <unistd.h>

#   include <cerrno>
#   include <cstring>
#   include <ctime>
#endif

// For sys::set_gid.
#if defined(HAVE_SETGID)
#   include <sys/types.h>
#   include <unistd.h>
#   include <grp.h>
#endif

// For sys::set_uid.
#if defined(HAVE_SETGID)
#   include <sys/types.h>
#   include <unistd.h>
#   include <pwd.h>
#endif

// For sys::username
#if defined(HAVE_GETLOGIN)
#   include <unistd.h>
#endif

#include "system.hpp"
#include "string_util.hpp"
#include "xdg.hpp"

namespace irccd {

namespace sys {

namespace {

/*
 * executable_path
 * ------------------------------------------------------------------
 *
 * Get the executable path.
 *
 * Example:
 *
 * /usr/local/bin/irccd -> /usr/local/bin
 */
std::string executable_path()
{
    std::string result;

#if defined(IRCCD_SYSTEM_LINUX)
    char path[PATH_MAX + 1] = {0};

    if (readlink("/proc/self/exe", path, sizeof (path) - 1) < 0)
        throw std::runtime_error(std::strerror(errno));

    result = path;
#elif defined(IRCCD_SYSTEM_FREEBSD) || defined(IRCCD_SYSTEM_DRAGONFLYBSD)
    int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
    char path[PATH_MAX + 1] = {0};
    size_t size = PATH_MAX;

    if (sysctl(mib, 4, path, &size, nullptr, 0) < 0)
        throw std::runtime_error(std::strerror(errno));

    result = path;
#elif defined(IRCCD_SYSTEM_MAC)
    char path[PROC_PIDPATHINFO_MAXSIZE + 1] = {0};

    if ((proc_pidpath(getpid(), path, sizeof (path) - 1)) == 0)
        throw std::runtime_error(std::strerror(errno));

    result = path;
#elif defined(IRCCD_SYSTEM_WINDOWS)
    char path[PATH_MAX + 1] = {0};

    if (GetModuleFileNameA(nullptr, path, sizeof (path) - 1) == 0)
        throw std::runtime_error("GetModuleFileName error");

    result = path;
#elif defined(IRCCD_SYSTEM_NETBSD)
    char path[4096 + 1] = {0};

#   if defined(KERN_PROC_PATHNAME)
    int mib[] = { CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME };
    int size = sizeof (path) - 1;

    if (sysctl(mib, 4, path, &size, nullptr, 0) < 0)
        throw std::runtime_error(std::strerror(errno));
#   else
    if (readlink("/proc/curproc/exe", path, sizeof (path) - 1) < 0)
        throw std::runtime_error(std::strerror(errno));
#   endif

    result = path;
#elif defined(IRCCD_SYSTEM_OPENBSD)
    char **paths, path[PATH_MAX + 1] = {0};
    int mib[] = { CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_ARGV };
    size_t length = 0;

    if (sysctl(mib, 4, 0, &length, 0, 0) < 0)
        throw std::runtime_error(std::strerror(errno));
    if (!(paths = static_cast<char**>(std::malloc(length))))
        throw std::runtime_error(std::strerror(errno));
    if (sysctl(mib, 4, paths, &length, 0, 0) < 0) {
        std::free(paths);
        throw std::runtime_error(std::strerror(errno));
    }

    realpath(paths[0], path);
    result = path;

    std::free(paths);
#endif

    return result;
}

/*
 * add_config_user_path
 * ------------------------------------------------------------------
 *
 * Referenced by: config_filenames.
 *
 * Add user config path.
 */
void add_config_user_path(std::vector<std::string>& result, const std::string& file)
{
    boost::filesystem::path path;

#if defined(IRCCD_SYSTEM_WINDOWS)
    char folder[MAX_PATH] = {0};

    if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, folder) == S_OK)
        path = folder + "\\irccd\\config";
    else
        path = ".";
#else
    try {
        path = xdg().config_home();
    } catch (...) {
        path = sys::env("HOME");
        path /= ".config";
    }

    path /= "irccd";
#endif

    path /= file;
    result.push_back(path.string());
}

/*
 * add_plugin_user_path
 * ------------------------------------------------------------------
 *
 * Referenced by: plugin_filenames.
 *
 * Like add add_config_user_path but for plugins.
 */
void add_plugin_user_path(std::vector<std::string>& result, const std::string& file)
{
    boost::filesystem::path path;

#if defined(IRCCD_SYSTEM_WINDOWS)
    char folder[MAX_PATH] = {0};

    if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, folder) == S_OK)
        path = folder + "\\irccd\\share";
#else
    try {
        path = xdg().data_home();
    } catch (...) {
        path = sys::env("HOME");
        path /= ".local/share";
    }

    path /= "irccd";
#endif

    path /= file;
    result.push_back(path.string());
}

/*
 * base_directory
 * ------------------------------------------------------------------
 *
 * Get the base program directory.
 *
 * If irccd has been compiled with relative paths, the base directory is
 * evaluated by climbing the `bindir' directory from the executable path.
 *
 * Otherwise, use the installation prefix.
 */
boost::filesystem::path base_directory()
{
    static const boost::filesystem::path bindir(WITH_BINDIR);
    static const boost::filesystem::path prefix(PREFIX);

    boost::filesystem::path path(".");

    if (bindir.is_relative()) {
        try {
            path = executable_path();
            path = path.parent_path();
        } catch (...) {
            path = "./";
        }

        // Compute relative base directory.
        for (auto len = std::distance(bindir.begin(), bindir.end()); len > 0; len--)
            path = path.parent_path();
        if (path.empty())
            path = ".";
    } else
        path = prefix;

    return path;
}

/*
 * add_system_path
 * ------------------------------------------------------------------
 *
 * Referenced by: config_filenames,
 *                plugin_filenames
 *
 * Add system path into the result list.
 */
void add_system_path(std::vector<std::string>& result,
                     const std::string& file,
                     const boost::filesystem::path& component)
{
    boost::filesystem::path path;

    if (component.is_absolute())
        path = component;
    else {
        path = base_directory();
        path /= component;
    }

    path /= file;
    result.push_back(path.string());
}

/*
 * system_directory
 * ------------------------------------------------------------------
 *
 * Compute the system wise directory path for the given component.
 *
 * Referenced by: cachedir,
 *                datadir,
 *                sysconfigdir
 */
std::string system_directory(const std::string& component)
{
    boost::filesystem::path path(component);

    if (path.is_relative()) {
        path = base_directory();
        path /= component;
    }

    return path.string();
}

} // !namespace

void set_program_name(std::string name) noexcept
{
#if defined(HAVE_SETPROGNAME)
    setprogname(name.c_str());
#else
    (void)name;
#endif
}

std::string name()
{
#if defined(IRCCD_SYSTEM_LINUX)
    return "Linux";
#elif defined(IRCCD_SYSTEM_WINDOWS)
    return "Windows";
#elif defined(IRCCD_SYSTEM_FREEBSD)
    return "FreeBSD";
#elif defined(IRCCD_SYSTEM_DRAGONFLYBSD)
    return "DragonFlyBSD";
#elif defined(IRCCD_SYSTEM_OPENBSD)
    return "OpenBSD";
#elif defined(IRCCD_SYSTEM_NETBSD)
    return "NetBSD";
#elif defined(IRCCD_SYSTEM_MAC)
    return "Mac";
#else
    return "Unknown";
#endif
}

std::string version()
{
#if defined(IRCCD_SYSTEM_WINDOWS)
    auto version = GetVersion();
    auto major = (DWORD)(LOBYTE(LOWORD(version)));
    auto minor = (DWORD)(HIBYTE(LOWORD(version)));

    return std::to_string(major) + "." + std::to_string(minor);
#else
    struct utsname uts;

    if (uname(&uts) < 0)
        throw std::runtime_error(std::strerror(errno));

    return std::string(uts.release);
#endif
}

uint64_t uptime()
{
#if defined(IRCCD_SYSTEM_WINDOWS)
    return ::GetTickCount64() / 1000;
#elif defined(IRCCD_SYSTEM_LINUX)
    struct sysinfo info;

    if (sysinfo(&info) < 0)
        throw std::runtime_error(std::strerror(errno));

    return info.uptime;
#elif defined(IRCCD_SYSTEM_MAC)
    struct timeval boottime;
    size_t length = sizeof (boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };

    if (sysctl(mib, 2, &boottime, &length, nullptr, 0) < 0)
        throw std::runtime_error(std::strerror(errno));

    time_t bsec = boottime.tv_sec, csec = time(nullptr);

    return difftime(csec, bsec);
#else
    struct timespec ts;

    if (clock_gettime(CLOCK_UPTIME, &ts) < 0)
        throw std::runtime_error(std::strerror(errno));

    return ts.tv_sec;
#endif
}

uint64_t ticks()
{
#if defined(IRCCD_SYSTEM_WINDOWS)
    _timeb tp;

    _ftime(&tp);

    return tp.time * 1000LL + tp.millitm;
#else
    struct timeval tp;

    gettimeofday(&tp, NULL);

    return tp.tv_sec * 1000LL + tp.tv_usec / 1000;
#endif
}

std::string home()
{
#if defined(IRCCD_SYSTEM_WINDOWS)
    char path[MAX_PATH];

    if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path) != S_OK)
        return "";

    return std::string(path);
#else
    return env("HOME");
#endif
}

std::string env(const std::string& var)
{
    auto value = std::getenv(var.c_str());

    if (value == nullptr)
        return "";

    return value;
}

#if defined(HAVE_SETUID)

void set_uid(const std::string& value)
{
    assert(value.c_str());

    uid_t id;

    if (string_util::is_int(value))
        id = std::stoi(value);
    else {
        auto pw = getpwnam(value.c_str());

        if (!pw)
            throw std::runtime_error(string_util::sprintf("uid %s not found", value));

        id = pw->pw_uid;
    }

    if (setuid(id) < 0)
        throw std::runtime_error(std::strerror(errno));
}

#endif

#if defined(HAVE_SETGID)

void set_gid(const std::string& value)
{
    assert(value.c_str());

    gid_t id;

    if (string_util::is_int(value))
        id = std::stoi(value);
    else {
        auto gr = getgrnam(value.c_str());

        if (!gr)
            throw std::runtime_error(string_util::sprintf("gid %s not found", value));

        id = gr->gr_gid;
    }

    if (setgid(id) < 0)
        throw std::runtime_error(std::strerror(errno));
}

#endif

std::string cachedir()
{
    return system_directory(WITH_CACHEDIR);
}

std::string datadir()
{
    return system_directory(WITH_DATADIR);
}

std::string sysconfigdir()
{
    return system_directory(WITH_SYSCONFDIR);
}

std::string username()
{
#if defined(HAVE_GETLOGIN)
    auto v = getlogin();

    if (v)
        return v;
#endif

    return "";
}

std::vector<std::string> config_filenames(std::string file)
{
    std::vector<std::string> result;

    add_config_user_path(result, file);
    add_system_path(result, file, WITH_SYSCONFDIR);

    return result;
}

std::vector<std::string> plugin_filenames(const std::string& name,
                                          const std::vector<std::string>& extensions)
{
    assert(!extensions.empty());

    std::vector<std::string> result;

    for (const auto& ext : extensions)
        add_plugin_user_path(result, name + ext);
    for (const auto& ext : extensions)
        add_system_path(result, name + ext, WITH_PLUGINDIR);

    return result;
}

} // !sys

} // !irccd

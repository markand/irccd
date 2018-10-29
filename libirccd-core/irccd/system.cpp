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

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>

#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/predef/os.h>

#include "sysconfig.hpp"

#if BOOST_OS_WINDOWS
#	include <sys/timeb.h>
#	include <shlobj.h>
#else
#	include <sys/utsname.h>
#	include <sys/types.h>
#	include <sys/param.h>
#	include <sys/time.h>
#	include <unistd.h>
#endif

#if BOOST_OS_LINUX
#	include <sys/sysinfo.h>
#endif

#if BOOST_OS_MACOS
#	include <sys/sysctl.h>
#	include <libproc.h>
#endif

#if defined(IRCCD_HAVE_GETLOGIN)
#	include <unistd.h>
#endif

#include "system.hpp"
#include "string_util.hpp"
#include "xdg.hpp"

namespace irccd::sys {

namespace {

// {{{ base_directory

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
auto base_directory() -> boost::filesystem::path
{
	static const boost::filesystem::path bindir(IRCCD_INSTALL_BINDIR);
	static const boost::filesystem::path prefix(IRCCD_INSTALL_PREFIX);

	boost::filesystem::path path(".");

	if (bindir.is_relative()) {
		try {
			path = boost::dll::program_location();
			path = path.parent_path();
		} catch (...) {
			path = ".";
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

// }}}

// {{{ system_directory

/*
 * system_directory
 * ------------------------------------------------------------------
 *
 * Compute the system directory path for the given component.
 *
 * Referenced by:
 *
 * - cachedir,
 * - datadir,
 * - sysconfigdir,
 * - plugindir.
 */
auto system_directory(const std::string& component) -> boost::filesystem::path
{
	boost::filesystem::path path(component);

	if (path.is_relative())
		path = base_directory() / component;

	return path.string();
}

// }}}

// {{{ user_config_directory

/*
 * user_config_directory
 * ------------------------------------------------------------------
 *
 * Get user configuration directory.
 *
 * Referenced by:
 *
 * - config_filenames.
 *
 * Requires:
 *
 * - Windows:
 *   - <shlobj.h>
 */
auto user_config_directory() -> boost::filesystem::path
{
	boost::filesystem::path path;

#if BOOST_OS_WINDOWS
	char folder[MAX_PATH] = {0};

	if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, folder) == S_OK) {
		path /= folder;
		path /= "\\irccd\\config";
	} else
		path = ".";
#else
	try {
		path = xdg().get_config_home();
	} catch (...) {
		path = sys::env("HOME");
		path /= ".config";
	}

	path /= "irccd";
#endif

	return path;
}

// }}}

// {{{ user_plugin_directory

/*
 * user_plugin_directory
 * ------------------------------------------------------------------
 *
 * Referenced by:
 *
 * - plugin_filenames.
 *
 * Requires:
 *
 * - Windows:
 *   - <shlobj.h>
 *
 * Like add user_config_directory but for plugins.
 */
auto user_plugin_directory() -> boost::filesystem::path
{
	boost::filesystem::path path;

#if BOOST_OS_WINDOWS
	char folder[MAX_PATH] = {0};

	if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, folder) == S_OK) {
		path /= folder;
		path /= "\\irccd\\share";
	}
#else
	try {
		path = xdg().get_data_home();
	} catch (...) {
		path = sys::env("HOME");
		path /= ".local/share";
	}

	path /= "irccd";
#endif

	return path / "plugins";
}

// }}}

} // !namespace

// {{{ set_program_name

void set_program_name(std::string name) noexcept
{
#if defined(IRCCD_HAVE_SETPROGNAME)
	static std::string save = name;

	setprogname(save.c_str());
#else
	(void)name;
#endif
}

// }}}

// {{{ name

auto name() -> std::string
{
#if BOOST_OS_LINUX
	return "Linux";
#elif BOOST_OS_WINDOWS
	return "Windows";
#elif BOOST_OS_BSD_FREE
	return "FreeBSD";
#elif BOOST_OS_BSD_DRAGONFLY
	return "DragonFlyBSD";
#elif BOOST_OS_BSD_OPEN
	return "OpenBSD";
#elif BOOST_OS_BSD_NET
	return "NetBSD";
#elif BOOST_OS_MACOS
	return "macOS";
#elif BOOST_OS_ANDROID
	return "Android";
#elif BOOST_OS_AIX
	return "Aix";
#elif BOOST_OS_HAIKU
	return "Haiku";
#elif BOOST_OS_IOS
	return "iOS";
#elif BOOST_OS_SOLARIS
	return "Solaris";
#else
	return "Unknown";
#endif
}

// }}}

// {{{ version

/*
 * Requires:
 *
 * - Windows:
 *   - <windows.h>
 * - Others:
 *   - <sys/utsname.h>
 */
auto version() -> std::string
{
#if BOOST_OS_WINDOWS
	const auto version = GetVersion();
	const auto major = (DWORD)(LOBYTE(LOWORD(version)));
	const auto minor = (DWORD)(HIBYTE(LOWORD(version)));

	return std::to_string(major) + "." + std::to_string(minor);
#else
	struct utsname uts;

	if (::uname(&uts) < 0)
		throw std::runtime_error(std::strerror(errno));

	return std::string(uts.release);
#endif
}

// }}}

// {{{ uptime

/*
 * Requires:
 *
 * - Windows:
 *   - <windows.h>
 * - Linux:
 *   - <sys/sysinfo.h>
 * - Mac:
 *   - <sys/types.h>
 *   - <sys/sysctl.h>
 * - Others:
 *   - <ctime>
 */
auto uptime() -> std::uint64_t
{
#if BOOST_OS_WINDOWS
	return ::GetTickCount64() / 1000;
#elif BOOST_OS_LINUX
	struct sysinfo info;

	if (sysinfo(&info) < 0)
		throw std::runtime_error(std::strerror(errno));

	return info.uptime;
#elif BOOST_OS_MACOS
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

// }}}

// {{{ ticks

/*
 * Requires:
 *
 * - Windows:
 *   - <sys/timeb.h>
 * - Others:
 *   - <sys/times.h>
 */
auto ticks() -> std::uint64_t
{
#if BOOST_OS_WINDOWS
	_timeb tp;

	_ftime(&tp);

	return tp.time * 1000LL + tp.millitm;
#else
	struct timeval tp;

	gettimeofday(&tp, NULL);

	return tp.tv_sec * 1000LL + tp.tv_usec / 1000;
#endif
}

// }}}

// {{{ home

/*
 * Requires:
 *
 * - Windows:
 *   - <shlobj.h>
 */
auto home() -> std::string
{
#if BOOST_OS_WINDOWS
	char path[MAX_PATH];

	if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path) != S_OK)
		return "";

	return std::string(path);
#else
	return env("HOME");
#endif
}

// }}}

// {{{ env

/*
 * Requires:
 *
 * - <cstdlib>
 */
auto env(const std::string& var) -> std::string
{
	const auto value = std::getenv(var.c_str());

	if (value == nullptr)
		return "";

	return value;
}

// }}}

// {{{ cachedir

auto cachedir() -> boost::filesystem::path
{
	return system_directory(IRCCD_INSTALL_LOCALSTATEDIR) / "cache/irccd";
}

// }}}

// {{{ datadir

auto datadir() -> boost::filesystem::path
{
	return system_directory(IRCCD_INSTALL_DATADIR);
}

// }}}

// {{{ sysconfdir

auto sysconfdir() -> boost::filesystem::path
{
	return system_directory(IRCCD_INSTALL_SYSCONFDIR) / "irccd";
}

// }}}

// {{{ plugindir

auto plugindir() -> boost::filesystem::path
{
	return system_directory(IRCCD_INSTALL_LIBDIR) / "irccd";
}

// }}}

// {{{ username

/*
 * Requires:
 *   - <unistd.h>
 */
auto username() -> std::string
{
#if defined(IRCCD_HAVE_GETLOGIN)
	auto v = getlogin();

	if (v)
		return v;
#endif

	return "";
}

// }}}

// {{{ config_filenames

auto config_filenames(std::string_view file) -> std::vector<std::string>
{
	// TODO: remove this once we can use std::filesystem.
	const std::string filename(file);

	return {
		(user_config_directory() / filename).string(),
		(sysconfdir() / filename).string()
	};
}

// }}}

// {{{ plugin_filenames

auto plugin_filenames(const std::string& name,
                      const std::vector<std::string>& extensions) -> std::vector<std::string>
{
	assert(!extensions.empty());

	std::vector<std::string> result;

	for (const auto& ext : extensions)
		result.push_back((user_plugin_directory() / (name + ext)).string());
	for (const auto& ext : extensions)
		result.push_back((plugindir() / (name + ext)).string());

	return result;
}

// }}}

} // !irccd::sys

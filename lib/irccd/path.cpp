/*
 * path.cpp -- special paths inside irccd
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

#include <algorithm>
#include <cassert>
#include <sstream>
#include <stdexcept>

#include <irccd-config.h>

#if defined(IRCCD_SYSTEM_WINDOWS)
#  include <Windows.h>
#  include <Shlobj.h>
#else
#  if defined(IRCCD_SYSTEM_LINUX)
#    include <limits.h>
#    include <unistd.h>
#    include <cerrno>
#    include <cstring>
#    include <stdexcept>
#  endif

#  if defined(IRCCD_SYSTEM_FREEBSD)
#    include <sys/types.h>
#    include <sys/sysctl.h>
#    include <limits.h>

#    include <array>
#    include <cerrno>
#    include <cstring>
#    include <stdexcept>
#  endif

#  if defined(IRCCD_SYSTEM_MAC)
#    include <cerrno>
#    include <cstring>
#    include <unistd.h>
#    include <libproc.h>
#  endif

#  include "xdg.h"
#endif

#include "fs.h"
#include "path.h"
#include "system.h"
#include "util.h"

namespace irccd {

namespace path {

namespace {

/*
 * Base program directory
 * ------------------------------------------------------------------
 *
 * This variable stores the program base directory. It is only enabled when irccd is relocatable because we can
 * retrieve the base directory by removing WITH_BINDIR.
 *
 * If it is empty, the program was not able to detect it (e.g. error, not supported).
 */

#if defined(IRCCD_RELOCATABLE)

std::string base;

#if defined(IRCCD_SYSTEM_WINDOWS)

std::string executablePath()
{
	std::string result;
	std::size_t size = PATH_MAX;
	
	result.resize(size);
	
	if (!(size = GetModuleFileNameA(nullptr, &result[0], size)))
		throw std::runtime_error("GetModuleFileName error");
	
	result.resize(size);
	
	return result;
}

#elif defined(IRCCD_SYSTEM_LINUX)

std::string executablePath()
{
	std::string result;
	
	result.resize(2048);
	
	auto size = readlink("/proc/self/exe", &result[0], 2048);
	
	if (size < 0)
		throw std::invalid_argument(std::strerror(errno));
	
	result.resize(size);
	
	return result;
}

#elif defined(IRCCD_SYSTEM_FREEBSD)

std::string executablePath()
{
	std::array<int, 4> mib{ { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 } };
	std::string result;
	std::size_t size = PATH_MAX + 1;
	
	result.resize(size);
	
	if (sysctl(mib.data(), 4, &result[0], &size, nullptr, 0) < 0)
		throw std::runtime_error(std::strerror(errno));
	
	result.resize(size);
	
	return result;
}

#elif defined(IRCCD_SYSTEM_MAC)

std::string executablePath()
{
	std::string result;
	std::size_t size = PROC_PIDPATHINFO_MAXSIZE;
	
	result.resize(size);
	
	if ((size = proc_pidpath(getpid(), &result[0], size)) == 0)
		throw std::runtime_error(std::strerror(errno));
	
	result.resize(size);
	
	return result;
}

#else

/*
 * TODO: add support for more systems here.
 *
 *  - NetBSD
 *  - OpenBSD
 */

std::string executablePath()
{
	return "";
}

#endif

#endif // !IRCCD_RELOCATABLE

/*
 * System paths
 * ------------------------------------------------------------------
 *
 * Compute system paths.
 *
 * Do not call any of these functions if irccd is relocatable and base is unset.
 */

std::string systemConfig()
{
#if defined(IRCCD_RELOCATABLE)
	assert(!base.empty());

	return base + WITH_CONFDIR;
#else
	return fs::isAbsolute(WITH_CONFDIR) ? WITH_CONFDIR : std::string(PREFIX) + fs::separator() + WITH_CONFDIR;
#endif
}

std::string systemData()
{
#if defined(IRCCD_RELOCATABLE)
	assert(!base.empty());

	return base + WITH_DATADIR;
#else
	return fs::isAbsolute(WITH_DATADIR) ? WITH_CONFDIR : std::string(PREFIX) + fs::separator() + WITH_DATADIR;
#endif
}

std::string systemCache()
{
#if defined(IRCCD_RELOCATABLE)
	assert(!base.empty());

	return base + WITH_CACHEDIR;
#else
	return fs::isAbsolute(WITH_CACHEDIR) ? WITH_CACHEDIR : std::string(PREFIX) + fs::separator() + WITH_CACHEDIR;
#endif
}

std::string systemPlugins()
{
#if defined(IRCCD_RELOCATABLE)
	assert(!base.empty());

	return base + WITH_PLUGINDIR;
#else
	return fs::isAbsolute(WITH_PLUGINDIR) ? WITH_PLUGINDIR : std::string(PREFIX) + fs::separator() + WITH_PLUGINDIR;
#endif
}

/*
 * User paths
 * ------------------------------------------------------------------
 *
 * Compute user paths.
 */

/*
 * userConfig
 * ---------------------------------------------------------
 *
 * Get the path directory to the user configuration. Example:
 *
 * Unix:
 *
 * XDG_CONFIG_HOME/irccd
 * HOME/.config/irccd
 *
 * Windows:
 *
 * CSIDL_LOCAL_APPDATA/irccd/config
 */
std::string userConfig()
{
	std::ostringstream oss;

#if defined(IRCCD_SYSTEM_WINDOWS)
	char path[MAX_PATH];

	if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path) != S_OK) {
		oss << "";
	} else {
		oss << path;
		oss << "\\irccd\\config\\";
	}
#else
	try {
		Xdg xdg;

		oss << xdg.configHome();
		oss << "/irccd/";
	} catch (const std::exception &) {
		const char *home = getenv("HOME");

		if (home != nullptr)
			oss << home;

		oss << "/.config/irccd/";
	}
#endif

	return oss.str();
}

/*
 * userData
 * --------------------------------------------------------
 *
 * Get the path to the data application.
 *
 * Unix:
 *
 * XDG_DATA_HOME/irccd
 * HOME/.local/share/irccd
 *
 * Windows:
 *
 * CSIDL_LOCAL_APPDATA
 */
std::string userData()
{
	std::ostringstream oss;

#if defined(IRCCD_SYSTEM_WINDOWS)
	char path[MAX_PATH];

	if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path) != S_OK)
		oss << "";
	else {
		oss << path;
		oss << "\\irccd\\share";
	}
#else
	try {
		Xdg xdg;

		oss << xdg.dataHome();
		oss << "/irccd/";
	} catch (const std::exception &) {
		const char *home = getenv("HOME");

		if (home != nullptr)
			oss << home;

		oss << "/.local/share/irccd/";
	}
#endif

	return oss.str();
}

/*
 * userCache
 * --------------------------------------------------------
 *
 * Directory for cache files.
 *
 * Unix:
 *
 * XDG_CACHE_HOME/irccd
 * HOME/.cache/irccd
 *
 * Windows:
 *
 * %TEMP% (e.g. C:\Users\<user>\AppData\Local\Temp)
 */
std::string userCache()
{
	std::ostringstream oss;

#if defined(IRCCD_SYSTEM_WINDOWS)
	char path[MAX_PATH + 1];

	GetTempPathA(sizeof (path), path);

	oss << path << "\\irccd\\";
#else
	try {
		Xdg xdg;

		oss << xdg.cacheHome();
		oss << "/irccd/";
	} catch (const std::exception &) {
		const char *home = getenv("HOME");

		if (home != nullptr)
			oss << home;

		oss << "/.cache/irccd/";
	}
#endif

	return oss.str();
}

/*
 * userPlugins
 * --------------------------------------------------------
 *
 * Path to the data + plugins.
 */
std::string userPlugins()
{
	return userData() + "/plugins/";
}

} // !namespace

#if defined(IRCCD_SYSTEM_WINDOWS)
const char Separator(';');
#else
const char Separator(':');
#endif

void setApplicationPath(const std::string &argv0)
{
#if defined(IRCCD_RELOCATABLE)
	try {
		base = executablePath();
	} catch (const std::exception &) {
		/*
		 * If an exception is thrown, that means the operatin system supports a function to get the executable
		 * path but it failed.
		 *
		 * TODO: show a waning
		 */
	}

	/*
	 * If we could not get the application path from the native function, check if argv[0] is an absolute path
	 * and use that from there.
	 *
	 * Otherwise, search from the PATH.
	 *
	 * In the worst case use current working directory.
	 */
	if (base.empty()) {
		if (fs::isAbsolute(argv0)) {
			base = argv0;
		} else {
			std::string name = fs::baseName(argv0);

			for (const auto &dir : util::split(sys::env("PATH"), std::string(1, Separator))) {
				std::string path = dir + fs::separator() + name;

				if (fs::exists(path)) {
					base = path;
					break;
				}
			}

			/* Not found in PATH? add dummy value */
			if (base.empty())
				base = std::string(".") + fs::separator() + WITH_BINDIR + fs::separator() + "dummy";
		}
	}

	/* Find bin/<progname> */
	auto pos = base.rfind(std::string(WITH_BINDIR) + fs::separator() + fs::baseName(base));

	if (pos != std::string::npos)
		base.erase(pos);

	/* Add trailing / or \\ for convenience */
	base = clean(base);

	assert(!base.empty());
#else
	(void)argv0;
#endif
}

std::string clean(std::string input)
{
	if (input.empty())
		return input;

	/* First, remove any duplicates */
	input.erase(std::unique(input.begin(), input.end(), [&] (char c1, char c2) {
		return c1 == c2 && (c1 == '/' || c1 == '\\');
	}), input.end());

	/* Add a trailing / or \\ */
	char c = input[input.length() - 1];
	if (c != '/' && c != '\\')
		input += fs::separator();

	/* Now converts all / to \\ for Windows and the opposite for Unix */
#if defined(IRCCD_SYSTEM_WINDOWS)
	std::replace(input.begin(), input.end(), '/', '\\');
#else
	std::replace(input.begin(), input.end(), '\\', '/');
#endif

	return input;
}

std::string get(Path path, Owner owner)
{
	assert(path >= PathConfig && path <= PathPlugins);
	assert(owner >= OwnerSystem && owner <= OwnerUser);

	std::string result;

	switch (owner) {
	case OwnerSystem:
		switch (path) {
		case PathCache:
			result = clean(systemCache());
			break;
		case PathConfig:
			result = clean(systemConfig());
			break;
		case PathData:
			result = clean(systemData());
			break;
		case PathPlugins:
			result = clean(systemPlugins());
			break;
		default:
			break;
		}
	case OwnerUser:
		switch (path) {
		case PathCache:
			result = clean(userCache());
			break;
		case PathConfig:
			result = clean(userConfig());
			break;
		case PathData:
			result = clean(userData());
			break;
		case PathPlugins:
			result = clean(userPlugins());
			break;
		default:
			break;
		}
	default:
		break;
	}

	return result;
}

std::vector<std::string> list(Path path)
{
	assert(path >= PathConfig && path <= PathPlugins);

	std::vector<std::string> list;

	switch (path) {
	case PathCache:
		list.push_back(clean(userCache()));
		list.push_back(clean(systemCache()));
		break;
	case PathConfig:
		list.push_back(clean(userConfig()));
		list.push_back(clean(systemConfig()));
		break;
	case PathData:
		list.push_back(clean(userData()));
		list.push_back(clean(systemData()));
		break;
	case PathPlugins:
		list.push_back(clean(fs::cwd()));
		list.push_back(clean(userPlugins()));
		list.push_back(clean(systemPlugins()));
		break;
	default:
		break;
	}

	return list;
}

} // !path

} // !irccd

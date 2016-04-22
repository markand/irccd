/*
 * system.cpp -- platform dependent functions for system inspection
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

#include <cstdlib>
#include <ctime>
#include <stdexcept>

#include "sysconfig.hpp"

#if defined(HAVE_SETPROGNAME)
#  include <cstdlib>
#endif

#if defined(IRCCD_SYSTEM_WINDOWS)
#  include <sys/types.h>
#  include <sys/timeb.h>
#  include <Windows.h>
#  include <Shlobj.h>
#else // All non Windows
#if defined(IRCCD_SYSTEM_MAC)
#  include <sys/sysctl.h>
#endif

#if defined(IRCCD_SYSTEM_LINUX)
#  include <sys/sysinfo.h>
#endif

#  include <sys/utsname.h>
#  include <sys/time.h>
#  include <sys/types.h>
#  include <unistd.h>

#  include <cerrno>
#  include <cstring>
#  include <stdexcept>
#  include <ctime>

#endif

/* For sys::setGid */
#if defined(HAVE_SETGID)
#  include <sys/types.h>
#  include <unistd.h>
#  include <grp.h>
#endif

/* For sys::setUid */
#if defined(HAVE_SETGID)
#  include <sys/types.h>
#  include <unistd.h>
#  include <pwd.h>
#endif

#include "fs.hpp"
#include "logger.hpp"
#include "system.hpp"
#include "util.hpp"

namespace irccd {

namespace sys {

namespace {

/*
 * setHelper
 * ------------------------------------------------------------------
 *
 * This is an helper for setting the uid or gid. It accepts both numeric and string uid and gid.
 *
 * If a name is specified as uid/group, the lookup function will be called and must be getpwname or
 * getgrname. Then, to get the id from the returned structure (struct passwd, struct group), the getter
 * function will return either pw_uid or gr_gid.
 *
 * Finally, when the id is resolved, the setter function (setuid, setgid) will be called.
 *
 * @param typeName the type of id (uid or gid)
 * @param value the value (numeric or name)
 * @param lookup the lookup function to resolve the name (getpwnam or getgrnam)
 * @param setter the function to apply the id (setuid or setgid)
 * @param getter the function to get the id from the informal structure
 */
template <typename IntType, typename LookupFunc, typename SetterFunc, typename FieldGetter>
void setHelper(const std::string &typeName, const std::string &value, LookupFunc lookup, SetterFunc setter, FieldGetter getter)
{
	IntType id;

	if (util::isNumber(value)) {
		id = std::stoi(value);
	} else {
		auto info = lookup(value.c_str());

		if (info == nullptr) {
			log::warning() << "irccd: invalid " << typeName << ": " << std::strerror(errno) << std::endl;
			return;
		} else {
			id = getter(info);

			log::debug() << "irccd: " << typeName << " " << value << " resolved to: " << id << std::endl;
		}
	}

	if (setter(id) < 0)
		log::warning() << "irccd: could not set " << typeName << ": " << std::strerror(errno) << std::endl;
	else
		log::info() << "irccd: setting " << typeName << " to " << value << std::endl;
}

/*
 * XXX: the setprogname() function keeps a pointer without copying it so when main's argv is modified, we're not using
 * the same name so create our own copy.
 */

std::string programNameCopy;

} // !namespace

void setProgramName(std::string name) noexcept
{
	programNameCopy = std::move(name);

#if defined(HAVE_SETPROGNAME)
	setprogname(programNameCopy.c_str());
#endif
}

const std::string &programName() noexcept
{
	return programNameCopy;
}

std::string name()
{
#if defined(IRCCD_SYSTEM_LINUX)
	return "Linux";
#elif defined(IRCCD_SYSTEM_WINDOWS)
	return "Windows";
#elif defined(IRCCD_SYSTEM_FREEBSD)
	return "FreeBSD";
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
	/* BSD */
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

std::string env(const std::string &var)
{
	auto value = std::getenv(var.c_str());

	if (value == nullptr)
		return "";

	return value;
}

#if defined(HAVE_SETUID)

void setUid(const std::string &value)
{
	setHelper<uid_t>("uid", value, &getpwnam, &setuid, [] (const struct passwd *pw) {
		return pw->pw_uid;
	});
}

#endif

#if defined(HAVE_SETGID)

void setGid(const std::string &value)
{
	setHelper<gid_t>("gid", value, &getgrnam, &setgid, [] (const struct group *gr) {
		return gr->gr_gid;
	});
}

#endif

} // !sys

} // !irccd

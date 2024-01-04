/*
 * jsapi-system.c -- Irccd.System API
 *
 * Copyright (c) 2013-2024 David Demelier <markand@malikania.fr>
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#       include <windows.h>
#elif defined(__linux__)
#       include <sys/sysinfo.h>
#elif defined(__APPLE__) || defined(__NetBSD__)
#       include <sys/types.h>
#       include <sys/sysctl.h>
#endif

#if !defined(_WIN32)
#       include <sys/utsname.h>
#endif

#include <duktape.h>

#include "jsapi-file.h"
#include "jsapi-system.h"

static int
nsleep(unsigned long ns)
{
	struct timespec ts = {
		.tv_sec = ns / 1000000000L,
		.tv_nsec = (ns % 1000000000L),
	};

	while (nanosleep(&ts, &ts) && errno == EINTR);

	return 0;
}

static int
System_env(duk_context *ctx)
{
	const char *name = duk_require_string(ctx, 0);
	const char *value = getenv(name);

	if (!value)
		duk_push_null(ctx);
	else
		duk_push_string(ctx, value);

	return 1;
}

static int
System_exec(duk_context *ctx)
{
	duk_push_uint(ctx, system(duk_require_string(ctx, 0)));

	return 1;
}

static int
System_home(duk_context *ctx)
{
#if defined(_WIN32)
	char path[MAX_PATH] = {0};

	SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);

	duk_push_string(ctx, path);
#else
	const char *home;

	if ((home = getenv("HOME")))
		duk_push_string(ctx, home);
	else
		duk_push_undefined(ctx);
#endif

	return 1;
}

static int
System_name(duk_context *ctx)
{
#if defined(__linux__)
	duk_push_string(ctx, "Linux");
#elif defined(_WIN32)
	duk_push_string(ctx, "Windows");
#elif defined(__FreeBSD__)
	duk_push_string(ctx, "FreeBSD");
#elif defined(__DragonFly__)
	duk_push_string(ctx, "DragonFlyBSD");
#elif defined(__OpenBSD__)
	duk_push_string(ctx, "OpenBSD");
#elif defined(__NetBSD__)
	duk_push_string(ctx, "NetBSD");
#elif defined(__APPLE__)
	duk_push_string(ctx, "macOS");
#elif defined(__ANDROID__)
	duk_push_string(ctx, "Android");
#elif defined(_AIX)
	duk_push_string(ctx, "Aix");
#elif defined(__HAIKU__)
	duk_push_string(ctx, "Haiku");
#elif defined(sun)
	duk_push_string(ctx, "Solaris");
#else
	duk_push_string(ctx, "Unknown");
#endif

	return 1;
}

static int
System_popen(duk_context *ctx)
{
	const char *cmd = duk_require_string(ctx, 0);
	const char *mode = duk_require_string(ctx, 1);
	FILE *fp;

	if (!(fp = popen(cmd, mode)))
		jsapi_system_raise(ctx);

	jsapi_file_push(ctx, NULL, fp, pclose);

	return 1;
}

static int
System_sleep(duk_context *ctx)
{
	return nsleep(duk_require_uint(ctx, 0) * 1000000000L);
}

static int
System_usleep(duk_context *ctx)
{
	return nsleep(duk_require_uint(ctx, 0) * 1000);
}

static int
System_uptime(duk_context *ctx)
{
#if defined(_WIN32)
	duk_push_uint(ctx, GetTickCount64() / 1000);
#elif defined(__linux__)
	struct sysinfo info;

	if (sysinfo(&info) < 0)
		jsapi_system_raise(ctx);

	duk_push_uint(ctx, info.uptime);
#elif defined(__APPLE__) || defined(__NetBSD__)
	struct timeval boottime;
	size_t length = sizeof (boottime);
	int mib[2] = { CTL_KERN, KERN_BOOTTIME };
	time_t bsec, csec;

	if (sysctl(mib, 2, &boottime, &length, NULL, 0) < 0)
		jsapi_system_raise(ctx);

	bsec = boottime.tv_sec;
	csec = time(NULL);

	duk_push_uint(ctx, difftime(csec, bsec));
#else
	struct timespec ts;

	/* Mostly POSIX compliant (CLOCK_UPTIME isn't POSIX though). */
	if (clock_gettime(CLOCK_UPTIME, &ts) < 0)
		jsapi_system_raise(ctx);

	duk_push_uint(ctx, ts.tv_sec);
#endif

	return 1;
}

static int
System_version(duk_context *ctx)
{
#if defined(_WIN32)
	DWORD version = GetVersion();
	DWORD major = (DWORD)(LOBYTE(LOWORD(version)));
	DWORD minor = (DWORD)(HIBYTE(LOWORD(version)));

	duk_push_sprintf(ctx, "%d.%d", (int)major, (int)minor);
#else
	struct utsname uts;

	if (uname(&uts) < 0)
		jsapi_system_raise(ctx);

	duk_push_string(ctx, uts.release);
#endif
	return 1;
}

static const duk_function_list_entry functions[] = {
	{ "env",        System_env,     1 },
	{ "exec",       System_exec,    1 },
	{ "home",       System_home,    0 },
	{ "name",       System_name,    0 },
	{ "popen",      System_popen,   2 },
	{ "sleep",      System_sleep,   1 },
	{ "uptime",     System_uptime,  0 },
	{ "usleep",     System_usleep,  1 },
	{ "version",    System_version, 0 },
	{ NULL,         NULL,           0 }
};

void
jsapi_system_raise(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_get_prop_string(ctx, -1, "SystemError");
	duk_remove(ctx, -2);
	duk_push_int(ctx, errno);
	duk_push_string(ctx, strerror(errno));
	duk_new(ctx, 2);

	(void)duk_throw(ctx);
}

void
jsapi_system_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, functions);
	duk_put_prop_string(ctx, -2, "System");
	duk_pop(ctx);
}

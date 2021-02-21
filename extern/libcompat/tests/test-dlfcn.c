/*
 * test-dlfcn.c -- test dlopen/dlsym/dlclose
 *
 * Copyright (c) 2020-2021 David Demelier <markand@malikania.fr>
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

#include <dlfcn.h>
#include <stdio.h>

#if defined(_WIN32)
#       define EXPORT __declspec(dllexport)
#else
#       define EXPORT
#endif

EXPORT int
hello(void)
{
	return 0;
}

int
main(void)
{
	void *handle;
	int (*func)(void);
	int ret = 1;

	if (!(handle = dlopen(NULL, RTLD_NOW)) ||
	    !(func = dlsym(handle, "hello"))) {
		fprintf(stderr, "%s\n", dlerror());
		goto end;
	}

	ret = func();

	dlclose(handle);

end:
	return ret;
}

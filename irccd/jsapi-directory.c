/*
 * jsapi-directory.c -- Irccd.Directory API
 *
 * Copyright (c) 2013-2026 David Demelier <markand@malikania.fr>
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

#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <unistd.h>

#include <duktape.h>

#include <irccd/util.h>

#include "jsapi-system.h"

enum {
	LIST_DOT = (1 << 0),
	LIST_DOT_DOT = (1 << 1)
};

struct cursor {
	char path[PATH_MAX];
	char entry[FILENAME_MAX];
	int recursive;
	void *data;
	int (*fn)(const struct cursor *);
};

struct finder {
	union {
		const char *pattern;
		regex_t regex;
	};
	struct cursor curs;
	void (*finish)(struct finder *);
};

static int
recursedir(int dirfd, struct cursor *cs)
{
	DIR *dp;
	struct dirent *entry;
	struct stat st;
	size_t entrylen;
	int childfd, ret = 0;

	if (!(dp = fdopendir(dirfd)))
		return -1;

	while ((entry = readdir(dp))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (fstatat(dirfd, entry->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0)
			continue;

		entrylen = strlen(entry->d_name);

		/*
		 * Append full path for the given entry.
		 * e.g. /foo/bar/ -> /foo/bar/quux.txt
		 */
		irc_util_strlcat(cs->path, entry->d_name, sizeof (cs->path));

		/* Go recursively if it's a directory and activated. */
		if (S_ISDIR(st.st_mode) && cs->recursive) {
			irc_util_strlcat(cs->path, "/", sizeof (cs->path));

			entrylen += 1;

			if ((childfd = openat(dirfd, entry->d_name, O_RDONLY | O_DIRECTORY)) < 0)
				continue;
			if ((ret = recursedir(childfd, cs))) {
				close(childfd);
				goto stop;
			}

			close(childfd);
		}

		irc_util_strlcpy(cs->entry, entry->d_name, sizeof (cs->entry));

		if (cs->fn(cs)) {
			ret = 1;
			goto stop;
		}

		cs->path[strlen(cs->path) - entrylen] = '\0';
	}
stop:
	closedir(dp);

	return ret;
}

static int
recurse(const char *path, struct cursor *cs)
{
	int fd, ret;
	size_t pathlen;

	if ((fd = open(path, O_RDONLY | O_DIRECTORY)) < 0)
		return -1;

	pathlen = strlen(path);

	if (irc_util_strlcpy(cs->path, path, sizeof (cs->path)) >= sizeof (cs->path))
		return errno = ENOMEM, -1;
	if (cs->path[pathlen - 1] != '/' && irc_util_strlcat(cs->path, "/", sizeof (cs->path)) >= sizeof (cs->path))
		return errno = ENOMEM, -1;

	ret = recursedir(fd, cs);
	close(fd);

	return ret;
}

static inline const char *
path(duk_context *ctx)
{
	const char *ret;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "path");

	if (duk_get_type(ctx, -1) != DUK_TYPE_STRING)
		(void)duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Directory object");

	ret = duk_get_string(ctx, -1);

	if (!ret || !ret[0])
		(void)duk_error(ctx, DUK_ERR_TYPE_ERROR, "directory object has empty path");

	duk_pop_n(ctx, 2);

	return ret;
}

static int
find_regex(const struct cursor *curs)
{
	const struct finder *fd = curs->data;

	return regexec(&fd->regex, curs->entry, 0, NULL, 0) == 0;
}

static int
find_name(const struct cursor *curs)
{
	const struct finder *fd = curs->data;

	return strcmp(curs->entry, fd->pattern) == 0;
}

static void
find_regex_finish(struct finder *fd)
{
	regfree(&fd->regex);
}

static int
find_helper(duk_context *ctx, const char *base, int recursive, int pattern_index)
{
	struct finder finder = {
		.curs = {
			.recursive = recursive,
			.data = &finder,
		}
	};
	int status;

	if (duk_is_string(ctx, pattern_index)) {
		finder.pattern = duk_get_string(ctx, pattern_index);
		finder.curs.fn = find_name;
	} else {
		/* Check if it's a RegExp object. */
		duk_get_global_string(ctx, "RegExp");

		if (!duk_instanceof(ctx, pattern_index, -1))
			/* TODO: better error. */
			return duk_error(ctx, DUK_ERR_TYPE_ERROR, "pattern arg error");

		duk_get_prop_string(ctx, pattern_index, "source");

		if (regcomp(&finder.regex, duk_to_string(ctx, -1), REG_EXTENDED) != 0)
			return duk_error(ctx, DUK_ERR_ERROR, "RegExp error");

		finder.curs.fn = find_regex;
		finder.finish = find_regex_finish;
		duk_pop_n(ctx, 2);
	}

	status = recurse(base, &finder.curs);

	if (finder.finish)
		finder.finish(&finder);

	if (status == 1)
		duk_push_string(ctx, finder.curs.path);
	else
		duk_push_null(ctx);

	return 1;
}

static int
rm(const struct cursor *curs)
{
	return remove(curs->path), 0;
}

static int
rm_helper(duk_context *ctx, const char *base, int recursive)
{
	struct stat st;
	struct cursor curs = {
		.recursive = 1,
		.fn = rm
	};

	if (stat(base, &st) < 0)
		return jsapi_system_raise(ctx), 0;
	else if (!S_ISDIR(st.st_mode)) {
		errno = ENOTDIR;
		return jsapi_system_raise(ctx), 0;
	}

	if (recursive)
		recurse(base, &curs);

	remove(base);

	return 0;
}

static inline void
mkpath(duk_context *ctx, const char *path)
{
	if (mkdir(path, 0755) < 0 && errno != EEXIST)
		jsapi_system_raise(ctx);
}

static int
Directory_prototype_find(duk_context *ctx)
{
	return find_helper(ctx, path(ctx), duk_opt_boolean(ctx, 1, 0), 0);
}

static int
Directory_prototype_remove(duk_context *ctx)
{
	return rm_helper(ctx, path(ctx), duk_opt_boolean(ctx, 0, 0));
}

static int
Directory_constructor(duk_context *ctx)
{
	const char *path = duk_require_string(ctx, 0);
	const int flags = duk_opt_int(ctx, 1, 0);
	int fd;
	DIR *dp;
	struct dirent *entry;
	struct stat st;

	if (!duk_is_constructor_call(ctx))
		return 0;

	duk_push_this(ctx);

	/* this.entries property. */
	duk_push_string(ctx, "entries");
	duk_push_array(ctx);

	if ((fd = open(path, O_RDONLY | O_DIRECTORY)) < 0)
		jsapi_system_raise(ctx);
	if (!(dp = fdopendir(fd))) {
		close(fd);
		jsapi_system_raise(ctx);
	}

	for (int i = 0; (entry = readdir(dp)); ) {
		if (strcmp(entry->d_name, ".") == 0 && !(flags & LIST_DOT))
			continue;
		if (strcmp(entry->d_name, "..") == 0 && !(flags & LIST_DOT_DOT))
			continue;

		duk_push_object(ctx);
		duk_push_string(ctx, entry->d_name);
		duk_put_prop_string(ctx, -2, "name");

		if (fstatat(fd, entry->d_name, &st, 0) < 0)
			duk_push_int(ctx, 0);
		else
			duk_push_int(ctx, st.st_mode & S_IFMT);

		duk_put_prop_string(ctx, -2, "type");
		duk_put_prop_index(ctx, -2, i++);
	}

	closedir(dp);
	close(fd);
	duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);

	/* this.path property. */
	duk_push_string(ctx, "path");
	duk_push_string(ctx, path);
	duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
	duk_pop(ctx);

	return 0;
}

static int
Directory_find(duk_context *ctx)
{
	const char *path = duk_require_string(ctx, 0);
	int recursive = duk_opt_boolean(ctx, 2, 0);

	return find_helper(ctx, path, recursive, 1);
}

static int
Directory_remove(duk_context* ctx)
{
	return rm_helper(ctx, duk_require_string(ctx, 0), duk_opt_boolean(ctx, 1, 0));
}

static int
Directory_mkdir(duk_context* ctx)
{
	char path[PATH_MAX] = {};

	/* Copy the directory to iterate over '/'. */
	irc_util_strlcpy(path, duk_require_string(ctx, 0), sizeof (path));

	for (char *p = p + 1; *p; ++p) {
		if (*p == '/') {
			*p = 0;
			mkpath(ctx, path);
			*p = '/';
		}
	}

	mkpath(ctx, path);

	return 0;
}

static const duk_function_list_entry methods[] = {
	{ "find",               Directory_prototype_find,       DUK_VARARGS     },
	{ "remove",             Directory_prototype_remove,     1               },
	{ NULL,                 NULL,                           0               }
};

static const duk_function_list_entry functions[] = {
	{ "find",               Directory_find,                 DUK_VARARGS     },
	{ "mkdir",              Directory_mkdir,                DUK_VARARGS     },
	{ "remove",             Directory_remove,               DUK_VARARGS     },
	{ NULL,                 NULL,                           0               }
};

static const duk_number_list_entry constants[] = {
	{ "Dot",                LIST_DOT        },
	{ "DotDot",             LIST_DOT_DOT    },
	{ "TypeFile",           S_IFREG         },
	{ "TypeDir",            S_IFDIR         },
	{ "TypeLink",           S_IFLNK         },
	{ "TypeBlock",          S_IFBLK         },
	{ "TypeCharacter",      S_IFCHR         },
	{ "TypeFifo",           S_IFIFO         },
	{ "TypeSocket",         S_IFSOCK        },
	{ "TypeUnknown",        0               },
	{ NULL,                 0               }
};

void
jsapi_directory_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_c_function(ctx, Directory_constructor, 2);
	duk_put_number_list(ctx, -1, constants);
	duk_put_function_list(ctx, -1, functions);
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, methods);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_put_prop_string(ctx, -2, "Directory");
	duk_pop(ctx);
}

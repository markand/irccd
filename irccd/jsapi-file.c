/*
 * jsapi-file.c -- Irccd.File API
 *
 * Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

#define _POSIX_C_SOURCE 202405L
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <duktape.h>

#include <irccd/util.h>

#include "jsapi-file.h"
#include "jsapi-system.h"

#define SIGNATURE DUK_HIDDEN_SYMBOL("Irccd.File")
#define PROTOTYPE DUK_HIDDEN_SYMBOL("Irccd.File.prototype")

struct file {
	char path[PATH_MAX];
	FILE *fp;
	int (*finalizer)(FILE *);
};

static int
read_until_eof(duk_context *ctx, struct file *file)
{
	char *ret = NULL, *newret, buf[BUFSIZ];
	size_t retsz = 0, nread;

	/*
	 * This function is shared with popen which can not be used with stat
	 * so read the file by small piece of chunks and concat them until we
	 * reach the end.
	 */
	while ((nread = fread(buf, 1, sizeof (buf), file->fp)) > 0) {
		if (!(newret = realloc(ret, retsz + nread))) {
			free(ret);
			jsapi_system_raise(ctx);
		}

		ret = newret;
		memcpy(ret + retsz, buf, nread);
		retsz += nread;
	}

	if (ferror(file->fp)) {
		free(ret);
		jsapi_system_raise(ctx);
	}

	duk_push_lstring(ctx, ret, retsz);
	free(ret);

	return 1;
}

static int
read_amount(duk_context *ctx, struct file *file, unsigned int amount)
{
	char *ret;
	size_t nread;

	if (!(ret = malloc(amount)))
		jsapi_system_raise(ctx);

	if ((nread = fread(ret, 1, amount, file->fp)) <= 0 || ferror(file->fp)) {
		free(ret);
		jsapi_system_raise(ctx);
	}

	duk_push_lstring(ctx, ret, nread);
	free(ret);

	return 1;
}

static void
push_stat(duk_context *ctx, const struct stat *st)
{
	duk_push_object(ctx);

	duk_push_int(ctx, st->st_atime);
	duk_put_prop_string(ctx, -2, "atime");
	duk_push_int(ctx, st->st_blksize);
	duk_put_prop_string(ctx, -2, "blksize");
	duk_push_int(ctx, st->st_blocks);
	duk_put_prop_string(ctx, -2, "blocks");
	duk_push_int(ctx, st->st_ctime);
	duk_put_prop_string(ctx, -2, "ctime");
	duk_push_int(ctx, st->st_dev);
	duk_put_prop_string(ctx, -2, "dev");
	duk_push_int(ctx, st->st_gid);
	duk_put_prop_string(ctx, -2, "gid");
	duk_push_int(ctx, st->st_ino);
	duk_put_prop_string(ctx, -2, "ino");
	duk_push_int(ctx, st->st_mode);
	duk_put_prop_string(ctx, -2, "mode");
	duk_push_int(ctx, st->st_mtime);
	duk_put_prop_string(ctx, -2, "mtime");
	duk_push_int(ctx, st->st_nlink);
	duk_put_prop_string(ctx, -2, "nlink");
	duk_push_int(ctx, st->st_rdev);
	duk_put_prop_string(ctx, -2, "rdev");
	duk_push_int(ctx, st->st_size);
	duk_put_prop_string(ctx, -2, "size");
	duk_push_int(ctx, st->st_uid);
	duk_put_prop_string(ctx, -2, "uid");
}

static struct file *
self(duk_context *ctx)
{
	struct file *file;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, SIGNATURE);
	file = duk_to_pointer(ctx, -1);
	duk_pop_2(ctx);

	if (!file)
		(void)duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a File object");

	return file;
}

static int
File_prototype_basename(duk_context *ctx)
{
	const struct file *file = self(ctx);

	duk_push_string(ctx, irc_util_basename(file->path));

	return 1;
}

static int
File_prototype_close(duk_context *ctx)
{
	struct file *file = self(ctx);

	if (file->fp) {
		file->finalizer(file->fp);
		file->fp = NULL;
	}

	return 0;
}

static int
File_prototype_dirname(duk_context *ctx)
{
	const struct file *file = self(ctx);

	duk_push_string(ctx, irc_util_dirname(file->path));

	return 1;
}

static int
File_prototype_lines(duk_context *ctx)
{
	struct file *file = self(ctx);
	char *line = NULL;
	size_t linesz = 0;

	if (!file->fp) {
		errno = EBADF;
		jsapi_system_raise(ctx);
	}

	duk_push_array(ctx);

	for (int i = 0; getline(&line, &linesz, file->fp) >= 0; ++i) {
		line[strcspn(line, "\r\n")] = 0;

		duk_push_string(ctx, line);
		duk_put_prop_index(ctx, -2, i);
	}

	free(line);

	if (ferror(file->fp))
		jsapi_system_raise(ctx);

	return 1;
}

static int
File_prototype_read(duk_context *ctx)
{
	struct file *file = self(ctx);
	unsigned int amount = duk_opt_uint(ctx, 0, -1);

	if (!file->fp) {
		errno = EBADF;
		jsapi_system_raise(ctx);
	}

	return amount == -1U
	    ? read_until_eof(ctx, file)
	    : read_amount(ctx, file, amount);
}

static int
File_prototype_readline(duk_context *ctx)
{
	const struct file *file = self(ctx);
	char *line = NULL;
	size_t linesz = 0;

	if (!file->fp) {
		errno = EBADF;
		jsapi_system_raise(ctx);
	}

	if (getline(&line, &linesz, file->fp) < 0) {
		free(line);

		if (feof(file->fp))
			return 0;

		jsapi_system_raise(ctx);
	}

	line[strcspn(line, "\r\n")] = 0;
	duk_push_string(ctx, line);
	free(line);

	return 1;
}

static int
File_prototype_remove(duk_context *ctx)
{
	if (remove(self(ctx)->path) < 0)
		jsapi_system_raise(ctx);

	return 0;
}

static int
File_prototype_seek(duk_context *ctx)
{
	const struct file *file = self(ctx);
	const int type = duk_require_int(ctx, 0);
	const long offset = duk_require_int(ctx, 1);

	if (!file->fp || fseek(file->fp, offset, type) < 0)
		jsapi_system_raise(ctx);

	return 0;
}

static int
File_prototype_stat(duk_context *ctx)
{
	const struct file *file = self(ctx);
	struct stat st;

	if (!file->fp || fstat(fileno(file->fp), &st) < 0)
		jsapi_system_raise(ctx);

	push_stat(ctx, &st);

	return 0;
}

static int
File_prototype_tell(duk_context *ctx)
{
	const struct file *file = self(ctx);
	long position;

	if (!file->fp || (position = ftell(file->fp)) < 0)
		return jsapi_system_raise(ctx), 0;

	duk_push_number(ctx, position);

	return 1;
}

static int
File_prototype_write(duk_context *ctx)
{
	const struct file *file = self(ctx);
	size_t datasz = 0, written;
	const char *data = duk_require_lstring(ctx, 0, &datasz);

	if (!file->fp)
		jsapi_system_raise(ctx);

	written = fwrite(data, 1, datasz, file->fp);

	if (ferror(file->fp))
		jsapi_system_raise(ctx);

	duk_push_uint(ctx, written);

	return 1;
}

static int
File_constructor(duk_context *ctx)
{
	const char *path = duk_require_string(ctx, 0);
	const char *mode = duk_require_string(ctx, 1);
	FILE *fp;
	struct file *file;

	if (!duk_is_constructor_call(ctx))
		return 0;

	if (!(fp = fopen(path, mode)))
		jsapi_system_raise(ctx);

	file = irc_util_calloc(1, sizeof (*file));
	file->fp = fp;
	file->finalizer = fclose;
	irc_util_strlcpy(file->path, path, sizeof (file->path));

	duk_push_this(ctx);
	duk_push_pointer(ctx, file);
	duk_put_prop_string(ctx, -2, SIGNATURE);
	duk_pop(ctx);

	return 0;
}

static int
File_destructor(duk_context *ctx)
{
	struct file *file;

	duk_get_prop_string(ctx, 0, SIGNATURE);

	if ((file = duk_to_pointer(ctx, -1)) && file->fp) {
		file->finalizer(file->fp);
		file->fp = NULL;
		free(file);
	}

	duk_pop(ctx);
	duk_del_prop_string(ctx, 0, SIGNATURE);

	return 0;
}

static int
File_basename(duk_context *ctx)
{
	const char *path = duk_require_string(ctx, 0);

	duk_push_string(ctx, irc_util_basename(path));

	return 1;
}

static int
File_dirname(duk_context *ctx)
{
	const char *path = duk_require_string(ctx, 0);

	duk_push_string(ctx, irc_util_dirname(path));

	return 1;
}

static int
File_exists(duk_context *ctx)
{
	const char *path = duk_require_string(ctx, 0);
	struct stat st;

	duk_push_boolean(ctx, stat(path, &st) == 0);

	return 1;
}

static int
File_remove(duk_context *ctx)
{
	const char *path = duk_require_string(ctx, 0);

	if (remove(path) < 0)
		jsapi_system_raise(ctx);

	return 0;
}

static int
File_stat(duk_context *ctx)
{
	const char *path = duk_require_string(ctx, 0);
	struct stat st;

	if (stat(path, &st) < 0)
		jsapi_system_raise(ctx);

	push_stat(ctx, &st);

	return 0;
}

static const duk_function_list_entry methods[] = {
	{ "basename",   File_prototype_basename,        0 },
	{ "close",      File_prototype_close,           0 },
	{ "dirname",    File_prototype_dirname,         0 },
	{ "lines",      File_prototype_lines,           0 },
	{ "read",       File_prototype_read,            1 },
	{ "readline",   File_prototype_readline,        0 },
	{ "remove",     File_prototype_remove,          0 },
	{ "seek",       File_prototype_seek,            2 },
	{ "stat",       File_prototype_stat,            0 },
	{ "tell",       File_prototype_tell,            0 },
	{ "write",      File_prototype_write,           1 },
	{ NULL,         NULL,                           0 }
};

static const duk_function_list_entry functions[] = {
	{ "basename",   File_basename,                  1 },
	{ "dirname",    File_dirname,                   1 },
	{ "exists",     File_exists,                    1 },
	{ "remove",     File_remove,                    1 },
	{ "stat",       File_stat,                      1 },
	{ NULL,         NULL,                           0 }
};

static const duk_number_list_entry constants[] = {
	{ "SeekCur",    SEEK_CUR                          },
	{ "SeekEnd",    SEEK_END                          },
	{ "SeekSet",    SEEK_SET                          },
	{ NULL,         0                                 }
};

void
jsapi_file_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_c_function(ctx, File_constructor, 2);
	duk_put_number_list(ctx, -1, constants);
	duk_put_function_list(ctx, -1, functions);
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, methods);
	duk_push_c_function(ctx, File_destructor, 1);
	duk_set_finalizer(ctx, -2);
	duk_dup(ctx, -1);
	duk_put_global_string(ctx, PROTOTYPE);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_put_prop_string(ctx, -2, "File");
	duk_pop(ctx);
}

void
jsapi_file_push(duk_context *ctx, const char *path, FILE *fp, int (*finalizer)(FILE *))
{
	assert(ctx);
	assert(fp);
	assert(finalizer);

	struct file file = {
		.fp = fp,
		.finalizer = finalizer
	};

	if (path)
		irc_util_strlcpy(file.path, path, sizeof (file.path));

	duk_push_object(ctx);
	duk_push_pointer(ctx, irc_util_memdup(&file, sizeof (file)));
	duk_put_prop_string(ctx, -2, SIGNATURE);
	duk_get_global_string(ctx, PROTOTYPE);
	duk_set_prototype(ctx, -2);
}

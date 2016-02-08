/*
 * js-file.h -- Irccd.File API
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

#include <cerrno>
#include <cstring>
#include <fstream>

#include <irccd-config.h>

#if defined(HAVE_STAT)
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include <filesystem.h>

#include "js-irccd.h"
#include "js-file.h"

#if defined(HAVE_STAT)

namespace irccd {

/*
 * js::File object for Javascript I/O
 * ------------------------------------------------------------------
 */

File::File(std::string path, const std::string &mode)
	: m_path(std::move(path))
	, m_destructor([] (std::FILE *fp) { std::fclose(fp); })
{
	if ((m_stream = std::fopen(m_path.c_str(), mode.c_str())) == nullptr)
		throw std::runtime_error(std::strerror(errno));
}

File::File(std::FILE *fp, std::function<void (std::FILE *)> destructor) noexcept
	: m_stream(fp)
	, m_destructor(std::move(destructor))
{
	assert(m_destructor != nullptr);
}

File::~File() noexcept
{
	close();
}

void File::close() noexcept
{
	if (m_stream) {
		m_destructor(m_stream);
		m_stream = nullptr;
	}
}

bool File::isClosed() noexcept
{
	return m_stream == nullptr;
}

void File::seek(long amount, long dir)
{
	if (std::fseek(m_stream, amount, dir) != 0)
		throw std::runtime_error(std::strerror(errno));
}

unsigned File::tell()
{
	long pos = std::ftell(m_stream);

	if (pos == -1L)
		throw std::runtime_error(std::strerror(errno));

	return pos;
}

std::string File::readline()
{
	std::string result;
	int ch;

	while ((ch = std::fgetc(m_stream)) != EOF && ch != '\n')
		result += ch;

	if (ch == EOF && std::ferror(m_stream))
		throw std::runtime_error(std::strerror(errno));

	return result;
}

std::string File::read(int amount)
{
	assert(amount != 0);

	std::string result;
	int ch;

	for (int i = 0; (ch = std::fgetc(m_stream)) != EOF; ) {
		result += ch;

		if (amount > 0 && ++i == amount)
			break;
	}

	if (ch == EOF && std::ferror(m_stream))
		throw std::runtime_error(std::strerror(errno));

	return result;
}

void File::write(const std::string &data)
{
	if (std::fwrite(data.c_str(), data.length(), 1, m_stream) != 1)
		throw std::runtime_error(std::strerror(errno));
}

bool File::eof() const noexcept
{
	return std::feof(m_stream);
}

/*
 * js::TypeInfo specialization for struct stat
 * ------------------------------------------------------------------
 */

namespace js {

template <>
class TypeInfo<struct stat> {
public:
	static void push(Context &ctx, const struct stat &st)
	{
		ctx.push(Object{});

#if defined(HAVE_STAT_ST_ATIME)
		ctx.putProperty(-2, "atime", static_cast<int>(st.st_atime));
#endif
#if defined(HAVE_STAT_ST_BLKSIZE)
		ctx.putProperty(-2, "blksize", static_cast<int>(st.st_blksize));
#endif
#if defined(HAVE_STAT_ST_BLOCKS)
		ctx.putProperty(-2, "blocks", static_cast<int>(st.st_blocks));
#endif
#if defined(HAVE_STAT_ST_CTIME)
		ctx.putProperty(-2, "ctime", static_cast<int>(st.st_ctime));
#endif
#if defined(HAVE_STAT_ST_DEV)
		ctx.putProperty(-2, "dev", static_cast<int>(st.st_dev));
#endif
#if defined(HAVE_STAT_ST_GID)
		ctx.putProperty(-2, "gid", static_cast<int>(st.st_gid));
#endif
#if defined(HAVE_STAT_ST_INO)
		ctx.putProperty(-2, "ino", static_cast<int>(st.st_ino));
#endif
#if defined(HAVE_STAT_ST_MODE)
		ctx.putProperty(-2, "mode", static_cast<int>(st.st_mode));
#endif
#if defined(HAVE_STAT_ST_MTIME)
		ctx.putProperty(-2, "mtime", static_cast<int>(st.st_mtime));
#endif
#if defined(HAVE_STAT_ST_NLINK)
		ctx.putProperty(-2, "nlink", static_cast<int>(st.st_nlink));
#endif
#if defined(HAVE_STAT_ST_RDEV)
		ctx.putProperty(-2, "rdev", static_cast<int>(st.st_rdev));
#endif
#if defined(HAVE_STAT_ST_SIZE)
		ctx.putProperty(-2, "size", static_cast<int>(st.st_size));
#endif
#if defined(HAVE_STAT_ST_UID)
		ctx.putProperty(-2, "uid", static_cast<int>(st.st_uid));
#endif
	}
};

} // !js

namespace {

#endif // !HAVE_STAT

/* --------------------------------------------------------
 * File methods
 * -------------------------------------------------------- */

/*
 * Method: File.prototype.basename()
 * --------------------------------------------------------
 *
 * Synonym of `Irccd.File.basename(path)` but with the path from the file.
 *
 * Returns:
 *   The base file name
 */
int methodBasename(js::Context &ctx)
{
	ctx.push(fs::baseName(ctx.self<js::Pointer<File>>()->path()));

	return 1;
}

/*
 * Method: File.prototype.close()
 * --------------------------------------------------------
 *
 * Force close of the file, automatically called when object is collected.
 */
int methodClose(js::Context &ctx)
{
	ctx.self<js::Pointer<File>>()->close();

	return 0;
}

/*
 * Method: File.prototype.dirname()
 * --------------------------------------------------------
 *
 * Synonym of `Irccd.File.dirname(path)` but with the path from the file.
 *
 * Returns:
 *   The base directory name
 */
int methodDirname(js::Context &ctx)
{
	ctx.push(fs::dirName(ctx.self<js::Pointer<File>>()->path()));

	return 1;
}

/*
 * Method: File.prototype.read(amount)
 * --------------------------------------------------------
 *
 * Read the specified amount of characters or the whole file.
 *
 * Arguments:
 *   - amount, the amount of characters or -1 to read all (Optional, default: -1)
 *
 * Returns:
 *   - The string
 *
 * Throws:
 *   - Any exception on error
 */
int methodRead(js::Context &ctx)
{
	auto amount = ctx.optional<int>(0, -1);
	auto self = ctx.self<js::Pointer<File>>();

	if (amount == 0 || self->isClosed())
		return 0;

	try {
		ctx.push(self->read(amount));
	} catch (const std::exception &) {
		ctx.raise(SystemError());
	}

	return 1;
}

/*
 * Method: File.prototype.readline()
 * --------------------------------------------------------
 *
 * Read the next line available.
 *
 * Returns:
 *   - The next line or undefined if eof
 * Throws:
 *   - Any exception on error
 */
int methodReadline(js::Context &ctx)
{
	try {
		auto file = ctx.self<js::Pointer<File>>();

		if (file->isClosed() || file->eof())
			return 0;

		ctx.push(file->readline());
	} catch (const std::exception &) {
		ctx.raise(SystemError());
	}

	return 1;
}

/*
 * Method: File.prototype.remove()
 * --------------------------------------------------------
 *
 * Synonym of File.remove(path) but with the path from the file.
 *
 * Throws:
 *   - Any exception on error
 */
int methodRemove(js::Context &ctx)
{
	if (::remove(ctx.self<js::Pointer<File>>()->path().c_str()) < 0)
		ctx.raise(SystemError());

	return 0;
}

/*
 * Method: File.prototype.seek(type, amount)
 * --------------------------------------------------------
 *
 * Sets the position in the file.
 *
 * Arguments:
 *   - type, the type of setting (File.SeekSet, File.SeekCur, File.SeekSet)
 *   - amount, the new offset
 * Throws:
 *   - Any exception on error
 */
int methodSeek(js::Context &ctx)
{
	auto type = ctx.require<int>(0);
	auto amount = ctx.require<int>(1);
	auto file = ctx.self<js::Pointer<File>>();

	if (file->isClosed())
		return 0;

	try {
		file->seek(amount, type);
	} catch (const std::exception &) {
		ctx.raise(SystemError());
	}

	return 0;
}

#if defined(HAVE_STAT)

/*
 * Method: File.prototype.stat() [optional]
 * --------------------------------------------------------
 *
 * Synonym of File.stat(path) but with the path from the file.
 *
 * Returns:
 *   - The stat information
 * Throws:
 *   - Any exception on error
 */
int methodStat(js::Context &ctx)
{
	struct stat st;
	auto file = ctx.self<js::Pointer<File>>();

	if (file->isClosed())
		return 0;

	if (::stat(file->path().c_str(), &st) < 0)
		ctx.raise(SystemError());

	ctx.push(st);

	return 1;
}

#endif // !HAVE_STAT

/*
 * Method: File.prototype.tell()
 * --------------------------------------------------------
 *
 * Get the actual position in the file.
 *
 * Returns:
 *   - The position
 * Throws:
 *   - Any exception on error
 */
int methodTell(js::Context &ctx)
{
	auto file = ctx.self<js::Pointer<File>>();

	if (file->isClosed())
		return 0;

	try {
		ctx.push(static_cast<int>(file->tell()));
	} catch (const std::exception &) {
		ctx.raise(SystemError());
	}

	return 1;
}

/*
 * Method: File.prototype.write(data)
 * --------------------------------------------------------
 *
 * Write some characters to the file.
 *
 * Arguments:
 *   - data, the character to write
 * Throws:
 *   - Any exception on error
 */
int methodWrite(js::Context &ctx)
{
	auto file = ctx.self<js::Pointer<File>>();

	if (file->isClosed())
		return 0;

	try {
		file->write(ctx.require<std::string>(0));
	} catch (const std::exception &) {
		ctx.raise(SystemError());
	}

	return 0;
}

const js::FunctionMap methods{
	{ "basename",	{ methodBasename,	0	} },
	{ "close",	{ methodClose,		0	} },
	{ "dirname",	{ methodDirname,	0	} },
	{ "read",	{ methodRead,		1	} },
	{ "readline",	{ methodReadline,	0	} },
	{ "remove",	{ methodRemove,		0	} },
	{ "seek",	{ methodSeek,		2	} },
#if defined(HAVE_STAT)
	{ "stat",	{ methodStat,		0	} },
#endif
	{ "tell",	{ methodTell,		0	} },
	{ "write",	{ methodWrite,		1	} },
};

/* --------------------------------------------------------
 * File "static" functions
 * -------------------------------------------------------- */

/*
 * Function: Irccd.File(path, mode) [constructor]
 * --------------------------------------------------------
 *
 * Open a file specified by path with the specified mode.
 *
 * Arguments:
 *   - path, the path to the file
 *   - mode, the mode, can be one of [abrwt]
 * Throws:
 *   - Any exception on error
 */
int constructor(js::Context &ctx)
{
	if (!duk_is_constructor_call(ctx))
		return 0;

	std::string path = ctx.require<std::string>(0);
	std::string mode = ctx.require<std::string>(1);

	try {
		ctx.construct(js::Pointer<File>{new File{path, mode}});
	} catch (const std::exception &) {
		ctx.raise(SystemError());
	}

	return 0;
}

/*
 * Function: Irccd.File.basename(path)
 * --------------------------------------------------------
 *
 * Return the file basename as specified in `basename(3)` C function.
 *
 * Arguments:
 *   - path, the path to the file
 * Returns:
 *   - the base name
 */
int functionBasename(js::Context &ctx)
{
	ctx.push(fs::baseName(ctx.require<std::string>(0)));

	return 1;
}

/*
 * Function: Irccd.File.dirname(path)
 * --------------------------------------------------------
 *
 * Return the file directory name as specified in `dirname(3)` C function.
 *
 * Arguments:
 *   - path, the path to the file
 * Returns:
 *   - the directory name
 */
int functionDirname(js::Context &ctx)
{
	ctx.push(fs::dirName(ctx.require<std::string>(0)));

	return 1;
}

/*
 * Function: Irccd.File.exists(path)
 * --------------------------------------------------------
 *
 * Check if the file exists.
 *
 * Arguments:
 *   - path, the path to the file
 * Returns:
 *   - true if exists
 * Throws:
 *   - Any exception if we don't have access
 */
int functionExists(js::Context &ctx)
{
	ctx.push(fs::exists(ctx.require<std::string>(0)));

	return 1;
}

/*
 * function Irccd.File.remove(path)
 * --------------------------------------------------------
 *
 * Remove the file at the specified path.
 *
 * Arguments:
 *   - path, the path to the file
 * Throws:
 *   - Any exception on error
 */
int functionRemove(js::Context &ctx)
{
	if (::remove(ctx.require<std::string>(0).c_str()) < 0)
		ctx.raise(SystemError());

	return 0;
}

#if defined(HAVE_STAT)

/*
 * function Irccd.File.stat(path) [optional]
 * --------------------------------------------------------
 *
 * Get file information at the specified path.
 *
 * Arguments:
 *   - path, the path to the file
 * Returns:
 *   - the stats information
 * Throws:
 *   - Any exception on error
 */
int functionStat(js::Context &ctx)
{
	struct stat st;

	if (::stat(ctx.require<std::string>(0).c_str(), &st) < 0)
		ctx.raise(SystemError{});

	ctx.push(st);

	return 1;
}

#endif // !HAVE_STAT

const js::FunctionMap functions{
	{ "basename",	{ functionBasename,	1			} },
	{ "dirname",	{ functionDirname,	1			} },
	{ "exists",	{ functionExists,	1			} },
	{ "remove",	{ functionRemove,	1			} },
#if defined(HAVE_STAT)
	{ "stat",	{ functionStat,		1			} },
#endif
};

const js::Map<int> constants{
	{ "SeekCur",	static_cast<int>(std::fstream::cur)	},
	{ "SeekEnd",	static_cast<int>(std::fstream::end)	},
	{ "SeekSet",	static_cast<int>(std::fstream::beg)	},
};

} // !namespace

void loadJsFile(js::Context &ctx)
{
	ctx.getGlobal<void>("Irccd");

	/* File object */
	ctx.push(js::Function{constructor, 2});
	ctx.push(constants);
	ctx.push(functions);

	/* Prototype */
	ctx.push(js::Object{});
	ctx.push(methods);
	ctx.putProperty(-1, "\xff""\xff""File", true);
	ctx.putProperty(-2, "prototype");

	/* Put File */
	ctx.putProperty(-2, "File");
	ctx.pop();
}

} // !irccd

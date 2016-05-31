/*
 * js-file.cpp -- Irccd.File API
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
#include <array>
#include <cassert>
#include <iterator>
#include <vector>

#include "sysconfig.hpp"

#if defined(HAVE_STAT)
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include "fs.hpp"
#include "mod-file.hpp"
#include "mod-irccd.hpp"
#include "plugin-js.hpp"

namespace irccd {

namespace {

const std::string Signature("\xff""\xff""irccd-file-ptr");
const std::string Prototype("\xff""\xff""irccd-file-prototype");

} // !namespace

#if defined(HAVE_STAT)

/*
 * duk::TypeInfo specialization for struct stat
 * ------------------------------------------------------------------
 */

namespace duk {

template <>
class TypeTraits<struct stat> {
public:
	static void push(Context *ctx, const struct stat &st)
	{
		duk::push(ctx, Object{});

#if defined(HAVE_STAT_ST_ATIME)
		duk::putProperty(ctx, -2, "atime", static_cast<int>(st.st_atime));
#endif
#if defined(HAVE_STAT_ST_BLKSIZE)
		duk::putProperty(ctx, -2, "blksize", static_cast<int>(st.st_blksize));
#endif
#if defined(HAVE_STAT_ST_BLOCKS)
		duk::putProperty(ctx, -2, "blocks", static_cast<int>(st.st_blocks));
#endif
#if defined(HAVE_STAT_ST_CTIME)
		duk::putProperty(ctx, -2, "ctime", static_cast<int>(st.st_ctime));
#endif
#if defined(HAVE_STAT_ST_DEV)
		duk::putProperty(ctx, -2, "dev", static_cast<int>(st.st_dev));
#endif
#if defined(HAVE_STAT_ST_GID)
		duk::putProperty(ctx, -2, "gid", static_cast<int>(st.st_gid));
#endif
#if defined(HAVE_STAT_ST_INO)
		duk::putProperty(ctx, -2, "ino", static_cast<int>(st.st_ino));
#endif
#if defined(HAVE_STAT_ST_MODE)
		duk::putProperty(ctx, -2, "mode", static_cast<int>(st.st_mode));
#endif
#if defined(HAVE_STAT_ST_MTIME)
		duk::putProperty(ctx, -2, "mtime", static_cast<int>(st.st_mtime));
#endif
#if defined(HAVE_STAT_ST_NLINK)
		duk::putProperty(ctx, -2, "nlink", static_cast<int>(st.st_nlink));
#endif
#if defined(HAVE_STAT_ST_RDEV)
		duk::putProperty(ctx, -2, "rdev", static_cast<int>(st.st_rdev));
#endif
#if defined(HAVE_STAT_ST_SIZE)
		duk::putProperty(ctx, -2, "size", static_cast<int>(st.st_size));
#endif
#if defined(HAVE_STAT_ST_UID)
		duk::putProperty(ctx, -2, "uid", static_cast<int>(st.st_uid));
#endif
	}
};

} // !duk

#endif // !HAVE_STAT

namespace {

/*
 * Anonymous helpers.
 * ------------------------------------------------------------------
 */

// Remove trailing \r for CRLF line style.
inline std::string clearCr(std::string input)
{
	if (input.length() > 0 && input.back() == '\r')
		input.pop_back();

	return input;
}

/*
 * File methods.
 * ------------------------------------------------------------------
 */

/*
 * Method: File.basename()
 * --------------------------------------------------------
 *
 * Synonym of `Irccd.File.basename(path)` but with the path from the file.
 *
 * Returns:
 *   The base name.
 */
duk::Ret methodBasename(duk::Context *ctx)
{
	duk::push(ctx, fs::baseName(duk::self<File *>(ctx)->path()));

	return 1;
}

/*
 * Method: File.close()
 * --------------------------------------------------------
 *
 * Force close of the file, automatically called when object is collected.
 */
duk::Ret methodClose(duk::Context *ctx)
{
	duk::self<File *>(ctx)->close();

	return 0;
}

/*
 * Method: File.dirname()
 * --------------------------------------------------------
 *
 * Synonym of `Irccd.File.dirname(path)` but with the path from the file.
 *
 * Returns:
 *   The directory name.
 */
duk::Ret methodDirname(duk::Context *ctx)
{
	duk::push(ctx, fs::dirName(duk::self<File *>(ctx)->path()));

	return 1;
}

/*
 * Method: File.lines()
 * --------------------------------------------------------
 *
 * Read all lines and return an array.
 *
 * Returns:
 *   An array with all lines.
 * Throws
 *   - Any exception on error.
 */
duk::Ret methodLines(duk::Context *ctx)
{
	duk::push(ctx, duk::Array{});

	std::FILE *fp = duk::self<File *>(ctx)->handle();
	std::string buffer;
	std::array<char, 128> data;
	std::int32_t i = 0;

	while (std::fgets(&data[0], data.size(), fp) != nullptr) {
		buffer += data.data();

		auto pos = buffer.find('\n');

		if (pos != std::string::npos) {
			duk::putProperty(ctx, -1, i++, clearCr(buffer.substr(0, pos)));
			buffer.erase(0, pos + 1);
		}
	}

	// Maybe an error in the stream.
	if (std::ferror(fp))
		duk::raise(ctx, SystemError());

	// Missing '\n' in end of file.
	if (!buffer.empty())
		duk::putProperty(ctx, -1, i++, clearCr(buffer));

	return 1;
}

/*
 * Method: File.read(amount)
 * --------------------------------------------------------
 *
 * Read the specified amount of characters or the whole file.
 *
 * Arguments:
 *   - amount, the amount of characters or -1 to read all (Optional, default: -1).
 * Returns:
 *   The string.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret methodRead(duk::Context *ctx)
{
	auto amount = duk::optional<int>(ctx, 0, -1);
	auto file = duk::self<File *>(ctx);

	if (amount == 0 || file->handle() == nullptr)
		return 0;

	try {
		std::string data;
		std::size_t total = 0;

		if (amount < 0) {
			std::array<char, 128> buffer;
			std::size_t nread;

			while (!std::feof(file->handle())) {
				nread = std::fread(&buffer[0], sizeof (buffer[0]), buffer.size(), file->handle());

				if (std::ferror(file->handle()))
					duk::raise(ctx, SystemError());

				std::copy(buffer.begin(), buffer.begin() + nread, std::back_inserter(data));
				total += nread;
			}
		} else {
			data.resize((std::size_t)amount);
			total = std::fread(&data[0], sizeof (data[0]), (std::size_t)amount, file->handle());

			if (std::ferror(file->handle()))
				duk::raise(ctx, SystemError());

			data.resize(total);
		}

		duk::push(ctx, data);
	} catch (const std::exception &) {
		duk::raise(ctx, SystemError());
	}

	return 1;
}

/*
 * Method: File.readline()
 * --------------------------------------------------------
 *
 * Read the next line available.
 *
 * Returns:
 *   The next line or undefined if eof.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret methodReadline(duk::Context *ctx)
{
	std::FILE *fp = duk::self<File *>(ctx)->handle();
	std::string result;

	if (fp == nullptr || std::feof(fp))
		return 0;
	for (int ch; (ch = std::fgetc(fp)) != EOF && ch != '\n'; )
		result += (char)ch;
	if (std::ferror(fp))
		duk::raise(ctx, SystemError());

	duk::push(ctx, clearCr(result));

	return 1;
}

/*
 * Method: File.remove()
 * --------------------------------------------------------
 *
 * Synonym of File.remove(path) but with the path from the file.
 *
 * Throws:
 *   - Any exception on error.
 */
duk::Ret methodRemove(duk::Context *ctx)
{
	if (::remove(duk::self<File *>(ctx)->path().c_str()) < 0)
		duk::raise(ctx, SystemError());

	return 0;
}

/*
 * Method: File.seek(type, amount)
 * --------------------------------------------------------
 *
 * Sets the position in the file.
 *
 * Arguments:
 *   - type, the type of setting (File.SeekSet, File.SeekCur, File.SeekSet),
 *   - amount, the new offset.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret methodSeek(duk::Context *ctx)
{
	auto type = duk::require<int>(ctx, 0);
	auto amount = duk::require<int>(ctx, 1);
	auto fp = duk::self<File *>(ctx)->handle();

	if (fp != nullptr && std::fseek(fp, amount, type) != 0)
		duk::raise(ctx, SystemError());

	return 0;
}

#if defined(HAVE_STAT)

/*
 * Method: File.stat() [optional]
 * --------------------------------------------------------
 *
 * Synonym of File.stat(path) but with the path from the file.
 *
 * Returns:
 *   The stat information.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret methodStat(duk::Context *ctx)
{
	struct stat st;
	auto file = duk::self<File *>(ctx);

	if (file->handle() == nullptr && ::stat(file->path().c_str(), &st) < 0)
		duk::raise(ctx, SystemError());
	else
		duk::push(ctx, st);

	return 1;
}

#endif // !HAVE_STAT

/*
 * Method: File.tell()
 * --------------------------------------------------------
 *
 * Get the actual position in the file.
 *
 * Returns:
 *   The position.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret methodTell(duk::Context *ctx)
{
	auto fp = duk::self<File *>(ctx)->handle();
	long pos;

	if (fp == nullptr)
		return 0;

	if ((pos = std::ftell(fp)) == -1L)
		duk::raise(ctx, SystemError());
	else
		duk::push(ctx, (int)pos);

	return 1;
}

/*
 * Method: File.write(data)
 * --------------------------------------------------------
 *
 * Write some characters to the file.
 *
 * Arguments:
 *   - data, the character to write.
 * Returns:
 *   The number of bytes written.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret methodWrite(duk::Context *ctx)
{
	std::FILE *fp = duk::self<File *>(ctx)->handle();
	std::string data = duk::require<std::string>(ctx, 0);

	if (fp == nullptr)
		return 0;

	std::size_t nwritten = std::fwrite(data.c_str(), 1, data.length(), fp);

	if (std::ferror(fp))
		duk::raise(ctx, SystemError());

	duk::push(ctx, (int)nwritten);

	return 1;
}

const duk::FunctionMap methods{
	{ "basename",	{ methodBasename,	0	} },
	{ "close",	{ methodClose,		0	} },
	{ "dirname",	{ methodDirname,	0	} },
	{ "lines",	{ methodLines,		0	} },
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

/*
 * File "static" functions
 * ------------------------------------------------------------------
 */

/*
 * Function: Irccd.File(path, mode) [constructor]
 * --------------------------------------------------------
 *
 * Open a file specified by path with the specified mode.
 *
 * Arguments:
 *   - path, the path to the file,
 *   - mode, the mode string.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret constructor(duk::Context *ctx)
{
	if (!duk::isConstructorCall(ctx))
		return 0;

	try {
		duk::construct(ctx, new File(duk::require<std::string>(ctx, 0), duk::require<std::string>(ctx, 1)));
	} catch (const std::exception &) {
		duk::raise(ctx, SystemError());
	}

	return 0;
}

/*
 * Function: Irccd.File() [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
duk::Ret destructor(duk::Context *ctx)
{
	delete static_cast<File *>(duk::getProperty<void *>(ctx, 0, Signature));

	duk::deleteProperty(ctx, 0, Signature);

	return 0;
}

/*
 * Function: Irccd.File.basename(path)
 * --------------------------------------------------------
 *
 * Return the file basename as specified in `basename(3)` C function.
 *
 * Arguments:
 *   - path, the path to the file.
 * Returns:
 *   The base name.
 */
duk::Ret functionBasename(duk::Context *ctx)
{
	duk::push(ctx, fs::baseName(duk::require<std::string>(ctx, 0)));

	return 1;
}

/*
 * Function: Irccd.File.dirname(path)
 * --------------------------------------------------------
 *
 * Return the file directory name as specified in `dirname(3)` C function.
 *
 * Arguments:
 *   - path, the path to the file.
 * Returns:
 *   The directory name.
 */
duk::Ret functionDirname(duk::Context *ctx)
{
	duk::push(ctx, fs::dirName( duk::require<std::string>(ctx, 0)));

	return 1;
}

/*
 * Function: Irccd.File.exists(path)
 * --------------------------------------------------------
 *
 * Check if the file exists.
 *
 * Arguments:
 *   - path, the path to the file.
 * Returns:
 *   True if exists.
 * Throws:
 *   - Any exception if we don't have access.
 */
duk::Ret functionExists(duk::Context *ctx)
{
	duk::push(ctx, fs::exists(duk::require<std::string>(ctx, 0)));

	return 1;
}

/*
 * function Irccd.File.remove(path)
 * --------------------------------------------------------
 *
 * Remove the file at the specified path.
 *
 * Arguments:
 *   - path, the path to the file.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret functionRemove(duk::Context *ctx)
{
	if (::remove(duk::require<std::string>(ctx, 0).c_str()) < 0)
		duk::raise(ctx, SystemError());

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
 *   - path, the path to the file.
 * Returns:
 *   The stat information.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret functionStat(duk::Context *ctx)
{
	struct stat st;

	if (::stat(duk::require<std::string>(ctx, 0).c_str(), &st) < 0)
		duk::raise(ctx, SystemError());

	duk::push(ctx, st);

	return 1;
}

#endif // !HAVE_STAT

const duk::FunctionMap functions{
	{ "basename",	{ functionBasename,	1			} },
	{ "dirname",	{ functionDirname,	1			} },
	{ "exists",	{ functionExists,	1			} },
	{ "remove",	{ functionRemove,	1			} },
#if defined(HAVE_STAT)
	{ "stat",	{ functionStat,		1			} },
#endif
};

const duk::Map<int> constants{
	{ "SeekCur",	SEEK_CUR },
	{ "SeekEnd",	SEEK_END },
	{ "SeekSet",	SEEK_SET },
};

} // !namespace

namespace duk {

void TypeTraits<File *>::construct(duk::Context *ctx, File *fp)
{
	assert(fp);

	duk::StackAssert sa(ctx);

	duk::push(ctx, duk::This());
	duk::putProperty<void *>(ctx, -1, Signature, fp);
	duk::pop(ctx);
}

void TypeTraits<File *>::push(duk::Context *ctx, File *fp)
{
	duk::StackAssert sa(ctx, 1);

	duk::push(ctx, duk::Object());
	duk::putProperty<void *>(ctx, -1, Signature, fp);
	duk::getGlobal<void>(ctx, Prototype);
	duk::setPrototype(ctx, -2);
}

File *TypeTraits<File *>::require(duk::Context *ctx, Index index)
{
	auto ptr = static_cast<File *>(duk::getProperty<void *>(ctx, index, Signature));

	if (!ptr)
		duk::raise(ctx, DUK_ERR_TYPE_ERROR, "not a File object");

	return ptr;
}

} // !duk

FileModule::FileModule() noexcept
	: Module("Irccd.File")
{
}

void FileModule::load(Irccd &, JsPlugin &plugin)
{
	duk::StackAssert sa(plugin.context());

	duk::getGlobal<void>(plugin.context(), "Irccd");
	duk::push(plugin.context(), duk::Function(constructor, 2));
	duk::put(plugin.context(), constants);
	duk::put(plugin.context(), functions);
	duk::push(plugin.context(), duk::Object{});
	duk::put(plugin.context(), methods);
	duk::push(plugin.context(), duk::Function(destructor, 1));
	duk::setFinalizer(plugin.context(), -2);
	duk::dup(plugin.context(), -1);
	duk::putGlobal(plugin.context(), Prototype);
	duk::putProperty(plugin.context(), -2, "prototype");
	duk::putProperty(plugin.context(), -2, "File");
	duk::pop(plugin.context());
}

} // !irccd

/*
 * js-directory.cpp -- Irccd.Directory API
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
#include <cstdio>
#include <cstring>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <string>

#include <irccd-config.h>

#include <directory.h>
#include <filesystem.h>
#include <path.h>

#include "js.h"
#include "js-irccd.h"

namespace irccd {

class JsDirectory : public Directory {
private:
	std::string m_path;

public:
	inline JsDirectory(std::string path, int flags)
		: Directory(path, flags)
		, m_path(std::move(path))
	{
	}

	inline const std::string &path() const noexcept
	{
		return m_path;
	}

	static inline const char *name() noexcept
	{
		return "\xff""\xff""Directory";
	}
};

namespace {

/*
 * Find an entry recursively (or not) in a directory using a predicate
 * which can be used to test for regular expression, equality.
 *
 * Do not use this function directly, use:
 *
 * - findName
 * - findRegex
 */
template <typename Pred>
std::string findPath(const std::string &base, bool recursive, Pred pred)
{
	/*
	 * For performance reason, we first iterate over all entries that are
	 * not directories to avoid going deeper recursively if the requested
	 * file is in the current directory.
	 */
	Directory directory(base);

	for (const DirectoryEntry &entry : directory)
		if (entry.type != DirectoryEntry::Dir && pred(entry.name))
			return base + entry.name;

	if (!recursive)
		throw std::out_of_range("entry not found");

	for (const DirectoryEntry &entry : directory) {
		if (entry.type == DirectoryEntry::Dir) {
			std::string next = base + entry.name + fs::Separator;
			std::string path = findPath(next, true, pred);

			if (!path.empty())
				return path;
		}
	}

	return "";
}

/*
 * Helper for finding by equality.
 */
std::string findName(std::string base, const std::string &pattern, bool recursive)
{
	return findPath(base, recursive, [&] (const std::string &entryname) -> bool {
		return pattern == entryname;
	});
}

/*
 * Helper for finding by regular expression
 */
std::string findRegex(const std::string &base, std::string pattern, bool recursive)
{
	std::regex regexp(pattern, std::regex::ECMAScript);
	std::smatch smatch;

	return findPath(base, recursive, [&] (const std::string &entryname) -> bool {
		return std::regex_match(entryname, smatch, regexp);
	});
}

/*
 * Generic find function for:
 *
 * - Directory.find
 * - Directory.prototype.find
 *
 * The patternIndex is the argument where to test if the argument is a regex or a string.
 */
int find(js::Context &ctx, std::string base, bool recursive, int patternIndex)
{
	base = path::clean(base);

	try {
		std::string path;

		if (ctx.is<std::string>(patternIndex)) {
			path = findName(base, ctx.get<std::string>(patternIndex), recursive);
		} else {
			/* Check if it's a valid RegExp object */
			ctx.getGlobal<void>("RegExp");

			bool isRegex = ctx.instanceof(patternIndex, -1);

			ctx.pop();

			if (isRegex)
				path = findRegex(base, ctx.getProperty<std::string>(patternIndex, "source"), recursive);
			else
				ctx.raise(js::TypeError{"pattern must be a string or a regex expression"});
		}

		if (path.empty())
			return 0;

		ctx.push(path);
	} catch (const std::exception &ex) {
		ctx.raise(js::Error{ex.what()});
	}

	return 1;
}

/*
 * Generic remove function for:
 *
 * - Directory.remove
 * - Directory.prototype.remove
 */
int remove(js::Context &ctx, const std::string &path, bool recursive)
{
	if (!recursive) {
		::remove(path.c_str());
	} else {
		try {
			Directory directory(path);

			for (const DirectoryEntry &entry : directory) {
				if (entry.type == DirectoryEntry::Dir) {
					(void)remove(ctx, path + fs::Separator + entry.name, true);
				} else {
					std::string filename = path + fs::Separator + entry.name;

					::remove(filename.c_str());
				}
			}

			::remove(path.c_str());
		} catch (const std::exception &) {
			// TODO: put the error in a log.
		}
	}

	return 0;
}

/*
 * Method: Directory.find(pattern, recursive)
 * --------------------------------------------------------
 *
 * Synonym of Directory.find(path, pattern, recursive) but the path is taken
 * from the directory object.
 *
 * Arguments:
 *   - pattern, the regular expression or file name,
 *   - recursive, set to true to search recursively (default: false).
 * Returns:
 *   The path to the file or undefined on errors or not found
 */
int methodFind(js::Context &ctx)
{
	return find(ctx, ctx.self<js::Pointer<JsDirectory>>()->path(), ctx.optional<bool>(1, false), 0);
}

/*
 * Method: Directory.remove(recursive)
 * --------------------------------------------------------
 *
 * Synonym of Directory.remove(recursive) but the path is taken from the
 * directory object.
 *
 * Arguments:
 *   - recursive, recursively or not (default: false).
 * Throws:
 *   - Any exception on error.
 */
int methodRemove(js::Context &ctx)
{
	return remove(ctx, ctx.self<js::Pointer<JsDirectory>>()->path(), ctx.optional<bool>(0, false));
}

const js::FunctionMap methods{
	{ "find",		{ methodFind,		DUK_VARARGS	} },
	{ "remove",		{ methodRemove,		1		} }
};

/* --------------------------------------------------------
 * Directory "static" functions
 * -------------------------------------------------------- */

/*
 * Function: Irccd.Directory(path, flags) [constructor]
 * --------------------------------------------------------
 *
 * Opens and read the directory at the specified path.
 *
 * Arguments:
 *   - path, the path to the directory,
 *   - flags, the optional flags (default: 0).
 * Throws:
 *   - Any exception on error
 */
int constructor(js::Context &ctx)
{
	if (!duk_is_constructor_call(ctx))
		return 0;

	try {
		JsDirectory *directory = new JsDirectory(ctx.require<std::string>(0), ctx.optional<int>(1, 0));

		ctx.construct(js::Pointer<JsDirectory>{directory});
		ctx.push(js::This{});
		ctx.push("count");
		ctx.push(directory->count());
		ctx.defineProperty(-3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
		ctx.push("path");
		ctx.push(directory->path());
		ctx.defineProperty(-3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);

		/* add entries */
		ctx.push("entries");
		ctx.push(js::Array{});

		int i = 0;
		for (const DirectoryEntry &entry : (*directory)) {
			ctx.push(js::Object{});
			ctx.putProperty(-1, "name", entry.name);
			ctx.putProperty(-1, "type", static_cast<int>(entry.type));
			ctx.putProperty(-2, i++);
		}

		ctx.defineProperty(-3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
	} catch (const std::exception &ex) {
		ctx.raise(SystemError(errno, ex.what()));
	}

	return 0;
}

/*
 * Function: Irccd.Directory.find(path, pattern, recursive)
 * --------------------------------------------------------
 *
 * Find an entry by a pattern or a regular expression.
 *
 * Arguments:
 *   - path, the base path,
 *   - pattern, the regular expression or file name,
 *   - recursive, set to true to search recursively (default: false).
 * Returns:
 *   The path to the file or undefined on errors or not found.
 */
int funcFind(js::Context &ctx)
{
	return find(ctx, ctx.require<std::string>(0), ctx.optional<bool>(2, false), 1);
}

/*
 * Function: Irccd.Directory.remove(path, recursive)
 * --------------------------------------------------------
 *
 * Remove the directory optionally recursively.
 *
 * Arguments:
 *   - path, the path to the directory,
 *   - recursive, recursively or not (default: false).
 * Throws:
 *   - Any exception on error.
 */
int funcRemove(js::Context &ctx)
{
	return remove(ctx, ctx.require<std::string>(0), ctx.optional<bool>(1, false));
}

/*
 * Function: Irccd.Directory.mkdir(path, mode = 0700)
 * --------------------------------------------------------
 *
 * Create a directory specified by path. It will create needed subdirectories
 * just like you have invoked mkdir -p.
 *
 * Arguments:
 *   - path, the path to the directory,
 *   - mode, the mode, not available on all platforms.
 * Throws:
 *   - Any exception on error.
 */
int funcMkdir(js::Context &ctx)
{
	try {
		fs::mkdir(ctx.require<std::string>(0), ctx.optional<int>(1, 0700));
	} catch (const std::exception &ex) {
		ctx.raise(SystemError{errno, ex.what()});
	}

	return 0;
}

const js::FunctionMap functions{
	{ "find",		{ funcFind,	DUK_VARARGS } },
	{ "mkdir",		{ funcMkdir,	DUK_VARARGS } },
	{ "remove",		{ funcRemove,	DUK_VARARGS } }
};

const js::Map<int> constants{
	{ "Dot",		static_cast<int>(Directory::Dot)		},
	{ "DotDot",		static_cast<int>(Directory::DotDot)		},
	{ "TypeUnknown",	static_cast<int>(DirectoryEntry::Unknown)	},
	{ "TypeDir",		static_cast<int>(DirectoryEntry::Dir)		},
	{ "TypeFile",		static_cast<int>(DirectoryEntry::File)		},
	{ "TypeLink",		static_cast<int>(DirectoryEntry::Link)		}
};

} // !namespace

void loadJsDirectory(js::Context &ctx) noexcept
{
	ctx.getGlobal<void>("Irccd");

	/* File object */
	ctx.push(js::Function{constructor, 2});
	ctx.push(constants);
	ctx.push(functions);
	ctx.putProperty(-1, "separator", std::string{fs::Separator});

	/* Prototype */
	ctx.push(js::Object{});
	ctx.push(methods);
	ctx.putProperty(-1, "\xff""\xff""Directory", true);
	ctx.putProperty(-2, "prototype");

	/* Put Directory */
	ctx.putProperty(-2, "Directory");
	ctx.pop();
}

} // !irccd

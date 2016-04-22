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

#include "fs.hpp"
#include "js.hpp"
#include "js-irccd.hpp"
#include "path.hpp"
#include "sysconfig.hpp"

namespace irccd {

namespace {

std::string path(duk::ContextPtr ctx)
{
	duk::push(ctx, duk::This{});
	duk::getProperty<void>(ctx, -1, "path");

	if (duk::type(ctx, -1) != DUK_TYPE_STRING)
		duk::raise(ctx, duk::TypeError("invalid this binding"));

	std::string ret = duk::get<std::string>(ctx, -1);

	if (ret.empty())
		duk::raise(ctx, duk::TypeError("invalid directory with empty path"));

	duk::pop(ctx, 2);

	return ret;
}

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
	auto entries = fs::readdir(base);

	for (const auto &entry : entries)
		if (entry.type != fs::Entry::Dir && pred(entry.name))
			return base + entry.name;

	if (!recursive)
		return "";

	for (const auto &entry : entries) {
		if (entry.type == fs::Entry::Dir) {
			std::string next = base + entry.name + fs::separator();
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
duk::Ret find(duk::ContextPtr ctx, std::string base, bool recursive, int patternIndex)
{
	base = path::clean(base);

	try {
		std::string path;

		if (duk::is<std::string>(ctx, patternIndex)) {
			path = findName(base, duk::get<std::string>(ctx, patternIndex), recursive);
		} else {
			/* Check if it's a valid RegExp object */
			duk::getGlobal<void>(ctx, "RegExp");

			bool isRegex = duk::instanceof(ctx, patternIndex, -1);

			duk::pop(ctx);

			if (isRegex)
				path = findRegex(base, duk::getProperty<std::string>(ctx, patternIndex, "source"), recursive);
			else
				duk::raise(ctx, duk::TypeError("pattern must be a string or a regex expression"));
		}

		if (path.empty())
			return 0;

		duk::push(ctx, path);
	} catch (const std::exception &ex) {
		duk::raise(ctx, duk::Error(ex.what()));
	}

	return 1;
}

/*
 * Generic remove function for:
 *
 * - Directory.remove
 * - Directory.prototype.remove
 */
duk::Ret remove(duk::ContextPtr ctx, const std::string &path, bool recursive)
{
	if (!fs::isDirectory(path))
		duk::raise(ctx, SystemError(EINVAL, "not a directory"));

	if (!recursive) {
#if defined(_WIN32)
		::RemoveDirectory(path.c_str());
#else
		::remove(path.c_str());
#endif
	} else {
		fs::rmdir(path.c_str());
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
 *   The path to the file or undefined if not found.
 * Throws:
 *   - Any exception on error.
 */
duk::Ret methodFind(duk::ContextPtr ctx)
{
	return find(ctx, path(ctx), duk::optional<bool>(ctx, 1, false), 0);
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
duk::Ret methodRemove(duk::ContextPtr ctx)
{
	return remove(ctx, path(ctx), duk::optional<bool>(ctx, 0, false));
}

const duk::FunctionMap methods{
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
duk::Ret constructor(duk::ContextPtr ctx)
{
	if (!duk_is_constructor_call(ctx))
		return 0;

	try {
		std::string path = duk::require<std::string>(ctx, 0);
		std::int8_t flags = duk::optional<int>(ctx, 1, 0);

		if (!fs::isDirectory(path))
			duk::raise(ctx, SystemError(EINVAL, "not a directory"));

		std::vector<fs::Entry> list = fs::readdir(path, flags);

		duk::push(ctx, duk::This{});
		duk::push(ctx, "count");
		duk::push(ctx, (int)list.size());
		duk::defineProperty(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
		duk::push(ctx, "path");
		duk::push(ctx, path);
		duk::defineProperty(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
		duk::push(ctx, "entries");
		duk::push(ctx, duk::Array{});

		for (unsigned i = 0; i < list.size(); ++i) {
			duk::push(ctx, duk::Object{});
			duk::putProperty(ctx, -1, "name", list[i].name);
			duk::putProperty(ctx, -1, "type", static_cast<int>(list[i].type));
			duk::putProperty(ctx, -2, (int)i);
		}

		duk::defineProperty(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);
	} catch (const std::exception &ex) {
		duk::raise(ctx, SystemError(errno, ex.what()));
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
duk::Ret funcFind(duk::ContextPtr ctx)
{
	return find(ctx, duk::require<std::string>(ctx, 0), duk::optional<bool>(ctx, 2, false), 1);
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
duk::Ret funcRemove(duk::ContextPtr ctx)
{
	return remove(ctx, duk::require<std::string>(ctx, 0), duk::optional<bool>(ctx, 1, false));
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
duk::Ret funcMkdir(duk::ContextPtr ctx)
{
	try {
		fs::mkdir(duk::require<std::string>(ctx, 0), duk::optional<int>(ctx, 1, 0700));
	} catch (const std::exception &ex) {
		duk::raise(ctx, SystemError(errno, ex.what()));
	}

	return 0;
}

const duk::FunctionMap functions{
	{ "find",		{ funcFind,	DUK_VARARGS } },
	{ "mkdir",		{ funcMkdir,	DUK_VARARGS } },
	{ "remove",		{ funcRemove,	DUK_VARARGS } }
};

const duk::Map<int> constants{
	{ "Dot",		static_cast<int>(fs::Dot)			},
	{ "DotDot",		static_cast<int>(fs::DotDot)			},
	{ "TypeUnknown",	static_cast<int>(fs::Entry::Unknown)		},
	{ "TypeDir",		static_cast<int>(fs::Entry::Dir)		},
	{ "TypeFile",		static_cast<int>(fs::Entry::File)		},
	{ "TypeLink",		static_cast<int>(fs::Entry::Link)		}
};

} // !namespace

void loadJsDirectory(duk::ContextPtr ctx) noexcept
{
	duk::StackAssert sa(ctx);

	duk::getGlobal<void>(ctx, "Irccd");
	duk::push(ctx, duk::Function{constructor, 2});
	duk::push(ctx, constants);
	duk::push(ctx, functions);
	duk::putProperty(ctx, -1, "separator", std::string{fs::separator()});
	duk::push(ctx, duk::Object{});
	duk::push(ctx, methods);
	duk::putProperty(ctx, -2, "prototype");
	duk::putProperty(ctx, -2, "Directory");
	duk::pop(ctx);
}

} // !irccd

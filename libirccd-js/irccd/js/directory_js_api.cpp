/*
 * directory_js_api.cpp -- Irccd.Directory API
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include <boost/predef/os.h>

#include <irccd/sysconfig.hpp>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <string>

#include <irccd/fs_util.hpp>

#include "directory_js_api.hpp"
#include "irccd_js_api.hpp"
#include "js_plugin.hpp"

namespace fs = boost::filesystem;

namespace irccd::js {

namespace {

// {{{ wrap

/*
 * Wrap the function and raise appropriate error in case of need.
 */
template <typename Handler>
auto wrap(duk_context* ctx, Handler handler) -> duk_ret_t
{
	try {
		return handler();
	} catch (const boost::system::system_error& ex) {
		duk::raise(ctx, ex);
	} catch (const std::system_error& ex) {
		duk::raise(ctx, ex);
	} catch (const std::exception& ex) {
		duk::raise(ctx, ex);
	}

	return 0;
}

// }}}

// {{{ path

/*
 * Get the path associated with the directory object as this.
 */
auto path(duk_context* ctx) -> std::string
{
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "path");

	if (duk_get_type(ctx, -1) != DUK_TYPE_STRING)
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Directory object");

	const auto ret = duk::get<std::string>(ctx, -1);

	if (ret.empty())
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "directory object has empty path");

	duk_pop_n(ctx, 2);

	return ret;
}

// }}}

// {{{ find

/*
 * Generic find function for:
 *
 * - Irccd.Directory.find
 * - Irccd.Directory.prototype.find
 *
 * The pattern_index is the argument where to test if the argument is a regex or
 * a string.
 */
auto find(duk_context* ctx, std::string base, bool recursive, int pattern_index) -> duk_ret_t
{
	/*
	 * Helper for checking if it's a valid RegExp object.
	 */
	const auto is_regex = [&] {
		duk_get_global_string(ctx, "RegExp");
		const auto result = duk_instanceof(ctx, pattern_index, -1);
		duk_pop(ctx);

		return result;
	};

	/*
	 * Helper for getting regex source.
	 */
	const auto pattern = [&] {
		duk_get_prop_string(ctx, pattern_index, "source");
		const auto pattern = duk_to_string(ctx, -1);
		duk_pop(ctx);

		return pattern;
	};

	std::string path;

	if (duk_is_string(ctx, pattern_index))
		path = fs_util::find(base, duk::get<std::string>(ctx, pattern_index), recursive);
	else if (is_regex())
		path = fs_util::find(base, pattern(), recursive);
	else
		throw duk::type_error("pattern must be a string or a regex expression");

	if (path.empty())
		return 0;

	return duk::push(ctx, path);
}

// }}}

// {{{ remove

/*
 * Generic remove function for:
 *
 * - Irccd.Directory.remove
 * - Irccd.Directory.prototype.remove
 */
auto remove(const std::string& path, bool recursive) -> duk_ret_t
{
	if (!boost::filesystem::is_directory(path))
		throw std::system_error(make_error_code(std::errc::invalid_argument));

	if (!recursive)
		boost::filesystem::remove(path);
	else
		boost::filesystem::remove_all(path);

	return 0;
}

// }}}

// {{{ Irccd.Directory.prototype.find

/*
 * Method: Irccd.Directory.prototype.find(pattern, recursive)
 * --------------------------------------------------------
 *
 * Synonym of Irccd.Directory.find(path, pattern, recursive) but the path is
 * taken from the directory object.
 *
 * Arguments:
 *   - pattern, the regular expression or file name,
 *   - recursive, set to true to search recursively (default: false).
 * Returns:
 *   The path to the file or undefined if not found.
 * Throws:
 *   - Irccd.SystemError on errors
 */
auto Directory_prototype_find(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		return find(ctx, path(ctx), duk::get<bool>(ctx, 1), 0);
	});
}

// }}}

// {{{ Irccd.Directory.prototype.remove

/*
 * Method: Irccd.Directory.prototype.remove(recursive)
 * --------------------------------------------------------
 *
 * Synonym of Directory.remove(recursive) but the path is taken from the
 * directory object.
 *
 * Arguments:
 *   - recursive, recursively or not (default: false).
 * Throws:
 *   - Irccd.SystemError on errors
 */
auto Directory_prototype_remove(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		return remove(path(ctx), duk::get<bool>(ctx, 0));
	});
}

// }}}

// {{{ Irccd.Directory [constructor]

/*
 * Function: Irccd.Directory(path) [constructor]
 * --------------------------------------------------------
 *
 * Opens and read the directory at the specified path.
 *
 * Arguments:
 *   - path, the path to the directory,
 * Throws:
 *   - Irccd.SystemError on errors
 */
auto Directory_constructor(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		if (!duk_is_constructor_call(ctx))
			return 0;

		const auto path = duk::require<std::string>(ctx, 0);

		if (!boost::filesystem::is_directory(path))
			throw std::system_error(make_error_code(std::errc::invalid_argument));

		duk_push_this(ctx);

		// 'entries' property.
		duk_push_string(ctx, "entries");
		duk_push_array(ctx);

		unsigned i = 0;
		for (const auto& entry : boost::filesystem::directory_iterator(path)) {
			duk_push_object(ctx);
			duk::push(ctx, entry.path().filename().string());
			duk_put_prop_string(ctx, -2, "name");
			duk_push_int(ctx, entry.status().type());
			duk_put_prop_string(ctx, -2, "type");
			duk_put_prop_index(ctx, -2, i++);
		}

		duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);

		// 'path' property.
		duk::push(ctx, "path");
		duk::push(ctx, path);
		duk_def_prop(ctx, -3, DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_HAVE_VALUE);

		return 0;
	});
}

// }}}

// {{{ Irccd.Directory.find

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
 * Throws:
 *   - Irccd.SystemError on errors
 */
auto Directory_find(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		return find(ctx, duk::require<std::string>(ctx, 0), duk::get<bool>(ctx, 2), 1);
	});
}

// }}}

// {{{ Irccd.Directory.remove

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
 *   - Irccd.SystemError on errors
 */
auto Directory_remove(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		return remove(duk::require<std::string>(ctx, 0), duk::get<bool>(ctx, 1));
	});
}

// }}}

// {{{ Irccd.Directory.mkdir

/*
 * Function: Irccd.Directory.mkdir(path, mode = 0700)
 * --------------------------------------------------------
 *
 * Create a directory specified by path. It will create needed subdirectories
 * just like you have invoked mkdir -p.
 *
 * Arguments:
 *   - path, the path to the directory,
 * Throws:
 *   - Irccd.SystemError on errors
 */
auto Directory_mkdir(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		boost::filesystem::create_directories(duk::require<std::string>(ctx, 0));

		return 0;
	});
}

// }}}

// {{{ definitions

const duk_function_list_entry methods[] = {
	{ "find",               Directory_prototype_find,       DUK_VARARGS     },
	{ "remove",             Directory_prototype_remove,     1               },
	{ nullptr,              nullptr,                        0               }
};

const duk_function_list_entry functions[] = {
	{ "find",               Directory_find,                 DUK_VARARGS     },
	{ "mkdir",              Directory_mkdir,                DUK_VARARGS     },
	{ "remove",             Directory_remove,               DUK_VARARGS     },
	{ nullptr,              nullptr,                        0               }
};

const duk_number_list_entry constants[] = {
	{ "TypeFile",           static_cast<int>(fs::regular_file)              },
	{ "TypeDir",            static_cast<int>(fs::directory_file)            },
	{ "TypeLink",           static_cast<int>(fs::symlink_file)              },
	{ "TypeBlock",          static_cast<int>(fs::block_file)                },
	{ "TypeCharacter",      static_cast<int>(fs::character_file)            },
	{ "TypeFifo",           static_cast<int>(fs::fifo_file)	                },
	{ "TypeSocket",         static_cast<int>(fs::socket_file)               },
	{ "TypeUnknown",	static_cast<int>(fs::type_unknown)              },
	{ nullptr,              0                                               }
};

// }}}

} // !namespace

// {{{ directory_js_api

auto directory_js_api::get_name() const noexcept -> std::string_view
{
	return "Irccd.Directory";
}

void directory_js_api::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
	duk::stack_guard sa(plugin->get_context());

	duk_get_global_string(plugin->get_context(), "Irccd");
	duk_push_c_function(plugin->get_context(), Directory_constructor, 2);
	duk_put_number_list(plugin->get_context(), -1, constants);
	duk_put_function_list(plugin->get_context(), -1, functions);

#if BOOST_OS_WINDOWS
	duk_push_string(plugin->get_context(), "\\");
#else
	duk_push_string(plugin->get_context(), "/");
#endif

	duk_put_prop_string(plugin->get_context(), -2, "separator");

	duk_push_object(plugin->get_context());
	duk_put_function_list(plugin->get_context(), -1, methods);
	duk_put_prop_string(plugin->get_context(), -2, "prototype");
	duk_put_prop_string(plugin->get_context(), -2, "Directory");
	duk_pop(plugin->get_context());
}

// }}}

} // !irccd::js

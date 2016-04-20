/*
 * path.hpp -- special paths inside irccd
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

#ifndef IRCCD_PATH_HPP
#define IRCCD_PATH_HPP

/**
 * @file path.hpp
 * @brief Path management.
 */

#include <string>
#include <vector>

namespace irccd {

namespace path {

/**
 * PATH separator, either : or ;.
 */
extern const char Separator;

/**
 * @enum Path
 * @brief Which special path to get
 */
enum Path {
	PathConfig,			//!< Configuration files
	PathData,			//!< Data directory
	PathCache,			//!< Cache files
	PathPlugins			//!< Path to the plugins
};

/**
 * @enum Owner
 * @brief For paths, get the installation path or the user ones
 */
enum Owner {
	OwnerSystem,			//!< System wide
	OwnerUser			//!< User
};

/**
 * This function must be called before at the beginning of the main.
 *
 * It use system dependant program path lookup if available and fallbacks to the path given as argument if any failure
 * was encoutered.
 *
 * @param argv0 the path to the executable (argv[0])
 */
void setApplicationPath(const std::string &argv0);

/**
 * Clean a path by removing any extra / or \ and add a trailing one.
 *
 * @param path the path
 * @return the updated path
 */
std::string clean(std::string path);

/**
 * Generic function for path retrievement.
 *
 * The path is always terminated by a trailing / or \\.
 *
 * @pre setApplicationPath must have been called
 * @param path the type of path
 * @param owner system or user wide
 * @return the path
 */
std::string get(Path path, Owner owner);

/**
 * Generic function for multiple paths.
 *
 * This function will add more directories than pathSystem*() and pathUser*() functions, for example
 * it will add some path if irccd is relocatable.
 *
 * @pre setApplicationPath must have been called
 * @param path the type of path
 * @return the list of preferred directories in order
 */
std::vector<std::string> list(Path path);

} // !path

} // !irccd

#endif // !IRCCD_PATH_HPP

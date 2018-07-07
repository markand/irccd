/*
 * system.hpp -- platform dependent functions for system inspection
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

#ifndef IRCCD_COMMON_SYSTEM_HPP
#define IRCCD_COMMON_SYSTEM_HPP

/**
 * \file system.hpp
 * \brief System dependant functions
 */

#include <cstdint>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "sysconfig.hpp"

namespace irccd {

/**
 * \brief Namespace for system functions.
 */
namespace sys {

/**
 * Set the program name, needed for some functions or some systems.
 *
 * \param name the program name
 */
void set_program_name(std::string name) noexcept;

/**
 * Get the system name.
 *
 * \return the name
 */
std::string name();

/**
 * Get the system version.
 *
 * \return the version
 */
std::string version();

/**
 * Get the number of seconds elapsed since the boottime.
 *
 * \return the number of seconds
 */
std::uint64_t uptime();

/**
 * Get the milliseconds elapsed since the application
 * startup.
 *
 * \return the milliseconds
 */
std::uint64_t ticks();

/**
 * Get an environment variable.
 *
 * \return the value or empty string
 */
std::string env(const std::string& var);

/**
 * Get home directory usually /home/foo
 *
 * \return the home directory
 */
std::string home();

/**
 * Get the cache directory as specified as compile time option
 * IRCCD_WITH_CACHEDIR, if the value is absolute, it is returned as-is.
 *
 * If the component is relative, it is evaluated using the binary executable
 * path.
 *
 * \return the evaluated cache directory.
 * \see datadir
 * \see configdir
 */
boost::filesystem::path cachedir();

/**
 * Like cachedir but for IRCCD_WITH_DATADIR.
 *
 * \return the evaluated data directory.
 * \see cachedir
 * \see datadir
 */
boost::filesystem::path datadir();

/**
 * Like cachedir but for IRCCD_WITH_SYSCONFIGDIR.
 *
 * \return the evaluated config directory.
 * \see cachedir
 * \see datadir
 * \note use config_filenames for irccd.conf, irccdctl.conf files
 */
 boost::filesystem::path sysconfdir();

/**
 * Like cachedir but for IRCCD_WITH_PLUGINDIR.
 *
 * \return the evaluated system plugin directory.
 * \see cachedir
 * \see datadir
 */
boost::filesystem::path plugindir();

/**
 * Get user account login or empty if not available.
 *
 * \return the user account name
 */
std::string username();

/**
 * Construct a list of paths to read configuration files from.
 *
 * This function does not test the presence of the files as a condition race
 * may occur.
 *
 * The caller is responsible of opening files for each path.
 *
 * \param file the filename to append for convenience
 * \return the list of paths to check in order
 */
std::vector<std::string> config_filenames(std::string file);

/**
 * Construct a list of paths for reading plugins.
 *
 * \param name the plugin id (without extension)
 * \param extensions the list of extensions supported
 */
std::vector<std::string> plugin_filenames(const std::string& name,
                                          const std::vector<std::string>& extensions);

} // !sys

} // !irccd

#endif // !IRCCD_COMMON_SYSTEM_HPP

/*
 * xdg.hpp -- XDG directory specifications
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_XDG_HPP
#define IRCCD_XDG_HPP

/**
 * \file xdg.hpp
 * \brief XDG directory specifications.
 * \author David Demelier <markand@malikana.fr>
 */

#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace irccd {

/**
 * \brief XDG directory specifications.
 *
 * Read and get XDG directories.
 *
 * This file should compiles on Windows to facilitate portability but its functions must not be used.
 */
class Xdg {
private:
    std::string m_configHome;
    std::string m_dataHome;
    std::string m_cacheHome;
    std::string m_runtimeDir;
    std::vector<std::string> m_configDirs;
    std::vector<std::string> m_dataDirs;

    bool isabsolute(const std::string &path) const noexcept
    {
        return path.length() > 0 && path[0] == '/';
    }

    std::vector<std::string> split(const std::string &arg) const
    {
        std::stringstream iss(arg);
        std::string item;
        std::vector<std::string> elems;

        while (std::getline(iss, item, ':'))
            if (isabsolute(item))
                elems.push_back(item);

        return elems;
    }

    std::string envOrHome(const std::string &var, const std::string &repl) const
    {
        auto value = std::getenv(var.c_str());

        if (value == nullptr || !isabsolute(value)) {
            auto home = std::getenv("HOME");

            if (home == nullptr)
                throw std::runtime_error("could not get home directory");

            return std::string(home) + "/" + repl;
        }

        return value;
    }

    std::vector<std::string> listOrDefaults(const std::string &var, const std::vector<std::string> &list) const
    {
        auto value = std::getenv(var.c_str());

        if (!value)
            return list;

        // No valid item at all? Use defaults.
        auto result = split(value);

        return (result.size() == 0) ? list : result;
    }

public:
    /**
     * Open an xdg instance and load directories.
     *
     * \throw std::runtime_error on failures
     */
    Xdg()
    {
        m_configHome    = envOrHome("XDG_CONFIG_HOME", ".config");
        m_dataHome      = envOrHome("XDG_DATA_HOME", ".local/share");
        m_cacheHome     = envOrHome("XDG_CACHE_HOME", ".cache");

        m_configDirs    = listOrDefaults("XDG_CONFIG_DIRS", { "/etc/xdg" });
        m_dataDirs      = listOrDefaults("XDG_DATA_DIRS", { "/usr/local/share", "/usr/share" });

        /*
         * Runtime directory is a special case and does not have a replacement, the application should manage
         * this by itself.
         */
        auto runtime = std::getenv("XDG_RUNTIME_DIR");
        if (runtime && isabsolute(runtime))
            m_runtimeDir = runtime;
    }

    /**
     * Get the config directory. ${XDG_CONFIG_HOME} or ${HOME}/.config
     *
     * \return the config directory
     */
    inline const std::string &configHome() const noexcept
    {
        return m_configHome;
    }

    /**
     * Get the data directory. ${XDG_DATA_HOME} or ${HOME}/.local/share
     *
     * \return the data directory
     */
    inline const std::string &dataHome() const noexcept
    {
        return m_dataHome;
    }

    /**
     * Get the cache directory. ${XDG_CACHE_HOME} or ${HOME}/.cache
     *
     * \return the cache directory
     */
    inline const std::string &cacheHome() const noexcept
    {
        return m_cacheHome;
    }

    /**
     * Get the runtime directory.
     *
     * There is no replacement for XDG_RUNTIME_DIR, if it is not set, an empty valus is returned and the user is
     * responsible of using something else.
     *
     * \return the runtime directory
     */
    inline const std::string &runtimeDir() const noexcept
    {
        return m_runtimeDir;
    }

    /**
     * Get the standard config directories. ${XDG_CONFIG_DIRS} or { "/etc/xdg" }
     *
     * \return the list of config directories
     */
    inline const std::vector<std::string> &configDirs() const noexcept
    {
        return m_configDirs;
    }

    /**
     * Get the data directories. ${XDG_DATA_DIRS} or { "/usr/local/share", "/usr/share" }
     *
     * \return the list of data directories
     */
    inline const std::vector<std::string> &dataDirs() const noexcept
    {
        return m_dataDirs;
    }
};

} // !irccd

#endif // !IRCCD_XDG_HPP

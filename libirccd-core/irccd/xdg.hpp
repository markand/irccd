/*
 * xdg.hpp -- XDG directory specifications
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
 * This file should compiles on Windows to facilitate portability but its
 * functions must not be used.
 */
class xdg {
private:
    std::string config_home_;
    std::string data_home_;
    std::string cache_home_;
    std::string runtime_dir_;
    std::vector<std::string> config_dirs_;
    std::vector<std::string> data_dirs_;

    inline bool is_absolute(const std::string& path) const noexcept
    {
        return path.length() > 0 && path[0] == '/';
    }

    std::vector<std::string> split(const std::string& arg) const
    {
        std::stringstream iss(arg);
        std::string item;
        std::vector<std::string> elems;

        while (std::getline(iss, item, ':')) {
            if (is_absolute(item))
                elems.push_back(item);
        }

        return elems;
    }

    std::string env_or_home(const std::string& var, const std::string& repl) const
    {
        auto value = std::getenv(var.c_str());

        if (value == nullptr || !is_absolute(value)) {
            auto home = std::getenv("HOME");

            if (home == nullptr)
                throw std::runtime_error("could not get home directory");

            return std::string(home) + "/" + repl;
        }

        return value;
    }

    std::vector<std::string> list_or_defaults(const std::string& var,
                                              const std::vector<std::string>& list) const
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
    xdg()
    {
        config_home_    = env_or_home("XDG_CONFIG_HOME", ".config");
        data_home_      = env_or_home("XDG_DATA_HOME", ".local/share");
        cache_home_     = env_or_home("XDG_CACHE_HOME", ".cache");

        config_dirs_    = list_or_defaults("XDG_CONFIG_DIRS", { "/etc/xdg" });
        data_dirs_      = list_or_defaults("XDG_DATA_DIRS", { "/usr/local/share", "/usr/share" });

        /*
         * Runtime directory is a special case and does not have a replacement,
         * the application should manage this by itself.
         */
        auto runtime = std::getenv("XDG_RUNTIME_DIR");

        if (runtime && is_absolute(runtime))
            runtime_dir_ = runtime;
    }

    /**
     * Get the config directory. ${XDG_CONFIG_HOME} or ${HOME}/.config
     *
     * \return the config directory
     */
    inline const std::string& config_home() const noexcept
    {
        return config_home_;
    }

    /**
     * Get the data directory. ${XDG_DATA_HOME} or ${HOME}/.local/share
     *
     * \return the data directory
     */
    inline const std::string& data_home() const noexcept
    {
        return data_home_;
    }

    /**
     * Get the cache directory. ${XDG_CACHE_HOME} or ${HOME}/.cache
     *
     * \return the cache directory
     */
    inline const std::string& cache_home() const noexcept
    {
        return cache_home_;
    }

    /**
     * Get the runtime directory.
     *
     * There is no replacement for XDG_RUNTIME_DIR, if it is not set, an empty
     * value is returned and the user is responsible of using something else.
     *
     * \return the runtime directory
     */
    inline const std::string& runtime_dir() const noexcept
    {
        return runtime_dir_;
    }

    /**
     * Get the standard config directories. ${XDG_CONFIG_DIRS} or { "/etc/xdg" }
     *
     * \return the list of config directories
     */
    inline const std::vector<std::string>& config_dirs() const noexcept
    {
        return config_dirs_;
    }

    /**
     * Get the data directories. ${XDG_DATA_DIRS} or { "/usr/local/share",
     * "/usr/share" }
     *
     * \return the list of data directories
     */
    inline const std::vector<std::string>& data_dirs() const noexcept
    {
        return data_dirs_;
    }
};

} // !irccd

#endif // !IRCCD_XDG_HPP

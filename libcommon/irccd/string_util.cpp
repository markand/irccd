/*
 * string_util.cpp -- string utilities
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

#include "sysconfig.hpp"

#if defined(HAVE_POPEN)
#   include <array>
#   include <cerrno>
#   include <cstring>
#   include <functional>
#   include <memory>
#endif

#include <iomanip>

#include "string_util.hpp"

using namespace std::string_literals;

namespace irccd {

namespace string_util {

namespace {

const std::unordered_map<std::string, int> irc_colors{
    { "white",      0   },
    { "black",      1   },
    { "blue",       2   },
    { "green",      3   },
    { "red",        4   },
    { "brown",      5   },
    { "purple",     6   },
    { "orange",     7   },
    { "yellow",     8   },
    { "lightgreen", 9   },
    { "cyan",       10  },
    { "lightcyan",  11  },
    { "lightblue",  12  },
    { "pink",       13  },
    { "grey",       14  },
    { "lightgrey",  15  }
};

const std::unordered_map<std::string, char> irc_attributes{
    { "bold",       '\x02'  },
    { "italic",     '\x09'  },
    { "strike",     '\x13'  },
    { "reset",      '\x0f'  },
    { "underline",  '\x15'  },
    { "underline2", '\x1f'  },
    { "reverse",    '\x16'  }
};

const std::unordered_map<std::string, unsigned> shell_colors{
    { "black",      30  },
    { "red",        31  },
    { "green",      32  },
    { "orange",     33  },
    { "blue",       34  },
    { "purple",     35  },
    { "cyan",       36  },
    { "white",      37  },
    { "default",    39  },
};

const std::unordered_map<std::string, unsigned> shell_attributes{
    { "bold",       1   },
    { "dim",        2   },
    { "underline",  4   },
    { "blink",      5   },
    { "reverse",    7   },
    { "hidden",     8   }
};

inline bool is_reserved(char token) noexcept
{
    return token == '#' || token == '@' || token == '$' || token == '!';
}

std::string subst_date(const std::string& text, const subst& params)
{
    std::ostringstream oss;

#if defined(HAVE_STD_PUT_TIME)
    oss << std::put_time(std::localtime(&params.time), text.c_str());
#else
    /*
     * Quick and dirty hack because old version of GCC does not have this
     * function.
     */
    char buffer[4096];

    std::strftime(buffer, sizeof (buffer) - 1, text.c_str(), std::localtime(&params.time));

    oss << buffer;
#endif

    return oss.str();
}

std::string subst_keywords(const std::string& content, const subst& params)
{
    auto value = params.keywords.find(content);

    if (value != params.keywords.end())
        return value->second;

    return "";
}

std::string subst_env(const std::string& content)
{
    auto value = std::getenv(content.c_str());

    if (value != nullptr)
        return value;

    return "";
}

std::string subst_irc_attrs(const std::string& content)
{
    auto list = split(content, ",");

    // @{} means reset.
    if (list.empty())
        return std::string(1, irc_attributes.at("reset"));

    std::ostringstream oss;

    // Remove useless spaces.
    std::transform(list.begin(), list.end(), list.begin(), strip);

    /*
     * 0: foreground
     * 1: background
     * 2-n: attributes
     */
    auto foreground = list[0];
    if (!foreground.empty() || list.size() >= 2) {
        // Color sequence.
        oss << '\x03';

        // Foreground.
        auto it = irc_colors.find(foreground);
        if (it != irc_colors.end())
            oss << it->second;

        // Background.
        if (list.size() >= 2 && (it = irc_colors.find(list[1])) != irc_colors.end())
            oss << "," << it->second;

        // Attributes.
        for (std::size_t i = 2; i < list.size(); ++i) {
            auto attribute = irc_attributes.find(list[i]);

            if (attribute != irc_attributes.end())
                oss << attribute->second;
        }
    }

    return oss.str();
}

std::string subst_shell_attrs(const std::string& content)
{
#if !defined(IRCCD_SYSTEM_WINDOWS)
    auto list = split(content, ",");

    if (list.empty())
        return "[0m";
    if (list.size() > 3)
        return "";

    std::vector<std::string> seq;

    /*
     * Shell sequence looks like this:
     *
     * [attributes;foreground;backgroundm
     */
    if (list.size() >= 3) {
        const auto it = shell_attributes.find(list[2]);

        if (it != shell_attributes.end())
            seq.push_back(std::to_string(it->second));
        else
            return "";
    }
    if (list.size() >= 1) {
        const auto it = shell_colors.find(list[0]);

        if (it != shell_colors.end())
            seq.push_back(std::to_string(it->second));
        else
            return "";
    }
    if (list.size() >= 2) {
        const auto it = shell_colors.find(list[1]);

        if (it != shell_colors.end())
            seq.push_back(std::to_string(it->second + 10));
        else
            return "";
    }

    std::ostringstream oss;

    oss << "[";
    oss << string_util::join(seq, ';');
    oss << "m";

    return oss.str();
#else
    return "";
#endif
}

std::string subst_shell(const std::string& command)
{
#if defined(HAVE_POPEN)
    std::unique_ptr<FILE, std::function<int (FILE*)>> fp(popen(command.c_str(), "r"), pclose);

    if (fp == nullptr)
        throw std::runtime_error(std::strerror(errno));

    std::string result;
    std::array<char, 128> buffer;
    std::size_t n;

    while ((n = std::fread(buffer.data(), 1, 128, fp.get())) > 0)
        result.append(buffer.data(), n);
    if (std::ferror(fp.get()))
        throw std::runtime_error(std::strerror(errno));

    // Erase final '\n'.
    auto it = result.find('\n');
    if (it != std::string::npos)
        result.erase(it);

    return result;
#else
    throw std::runtime_error("shell template not available");
#endif
}

std::string substitute(std::string::const_iterator& it,
                       std::string::const_iterator& end,
                       char token,
                       const subst& params)
{
    assert(is_reserved(token));

    std::string content, value;

    if (it == end)
        return "";

    while (it != end && *it != '}')
        content += *it++;

    if (it == end || *it != '}')
        throw std::invalid_argument("unclosed "s + token + " construct"s);

    it++;

    // Create default original value if flag is disabled.
    value = std::string(1, token) + "{"s + content + "}"s;

    switch (token) {
    case '#':
        if ((params.flags & subst_flags::keywords) == subst_flags::keywords)
            value = subst_keywords(content, params);
        break;
    case '$':
        if ((params.flags & subst_flags::env) == subst_flags::env)
            value = subst_env(content);
        break;
    case '@':
        if ((params.flags & subst_flags::irc_attrs) == subst_flags::irc_attrs)
            value = subst_irc_attrs(content);
        else if ((params.flags & subst_flags::shell_attrs) == subst_flags::shell_attrs)
            value = subst_shell_attrs(content);
        break;
    case '!':
        if ((params.flags & subst_flags::shell) == subst_flags::shell)
            value = subst_shell(content);
        break;
    default:
        break;
    }

    return value;
}

} // !namespace

std::string format(std::string text, const subst& params)
{
    /*
     * Change the date format before anything else to avoid interpolation with
     * keywords and user input.
     */
    if ((params.flags & subst_flags::date) == subst_flags::date)
        text = subst_date(text, params);

    std::ostringstream oss;

    for (auto it = text.cbegin(), end = text.cend(); it != end; ) {
        auto token = *it;

        // Is the current character a reserved token or not?
        if (!is_reserved(token)) {
            oss << *it++;
            continue;
        }

        // The token was at the end, just write it and return now.
        if (++it == end) {
            oss << token;
            continue;
        }

        // The token is declaring a template variable, substitute it.
        if (*it == '{') {
            oss << substitute(++it, end, token, params);
            continue;
        }

        /*
         * If the next token is different from the previous one, just let the
         * next iteration parse the string because we can have the following
         * constructs.
         *
         * "@#{var}" -> "@value"
         */
        if (*it != token) {
            oss << token;
            continue;
        }

        /*
         * Write the token only if it's not a variable because at this step we
         * may have the following constructs.
         *
         * "##" -> "##"
         * "##hello" -> "##hello"
         * "##{hello}" -> "#{hello}"
         */
        if (++it == end)
            oss << token << token;
        else if (*it == '{')
            oss << token;
    }

    return oss.str();
}

std::string strip(std::string str)
{
    auto test = [] (char c) { return !std::isspace(c); };

    str.erase(str.begin(), std::find_if(str.begin(), str.end(), test));
    str.erase(std::find_if(str.rbegin(), str.rend(), test).base(), str.end());

    return str;
}

std::vector<std::string> split(const std::string& list, const std::string& delimiters, int max)
{
    std::vector<std::string> result;
    std::size_t next = -1, current;
    int count = 1;
    bool finished = false;

    if (list.empty())
        return result;

    do {
        std::string val;

        current = next + 1;
        next = list.find_first_of(delimiters, current);

        // split max, get until the end.
        if (max >= 0 && count++ >= max) {
            val = list.substr(current, std::string::npos);
            finished = true;
        } else {
            val = list.substr(current, next - current);
            finished = next == std::string::npos;
        }

        result.push_back(val);
    } while (!finished);

    return result;
}

bool is_boolean(std::string value) noexcept
{
    std::transform(value.begin(), value.end(), value.begin(), [] (auto c) {
        return toupper(c);
    });

    return value == "1" || value == "YES" || value == "TRUE" || value == "ON";
}

} // !string_util

} // !util

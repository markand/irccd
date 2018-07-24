/*
 * journal_server.cpp -- journaled server that logs every command
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

#include "journal_server.hpp"

namespace irccd {

void journal_server::connect(connect_handler) noexcept
{
    cqueue_.push_back({
        { "command",    "connect"               },
    });
}

void journal_server::disconnect() noexcept
{
    cqueue_.push_back({
        { "command",    "disconnect"            },
    });
}

void journal_server::invite(std::string_view target, std::string_view channel)
{
    cqueue_.push_back({
        { "command",    "invite"                },
        { "target",     std::string(target)     },
        { "channel",    std::string(channel)    }
    });
}

void journal_server::join(std::string_view channel, std::string_view password)
{
    cqueue_.push_back({
        { "command",    "join"                  },
        { "channel",    std::string(channel)    },
        { "password",   std::string(password)   }
    });
}

void journal_server::kick(std::string_view target, std::string_view channel, std::string_view reason)
{
    cqueue_.push_back({
        { "command",    "kick"                  },
        { "target",     std::string(target)     },
        { "channel",    std::string(channel)    },
        { "reason",     std::string(reason)     }
    });
}

void journal_server::me(std::string_view target, std::string_view message)
{
    cqueue_.push_back({
        { "command",    "me"                    },
        { "target",     std::string(target)     },
        { "message",    std::string(message)    }
    });
}

void journal_server::message(std::string_view target, std::string_view message)
{
    cqueue_.push_back({
        { "command",    "message"               },
        { "target",     std::string(target)     },
        { "message",    std::string(message)    }
    });
}

void journal_server::mode(std::string_view channel,
                          std::string_view mode,
                          std::string_view limit,
                          std::string_view user,
                          std::string_view mask)
{
    cqueue_.push_back({
        { "command",    "mode"                  },
        { "channel",    std::string(channel)    },
        { "mode",       std::string(mode)       },
        { "limit",      std::string(limit)      },
        { "user",       std::string(user)       },
        { "mask",       std::string(mask)       }
    });
}

void journal_server::names(std::string_view channel)
{
    cqueue_.push_back({
        { "command",    "names"                 },
        { "channel",    std::string(channel)    }
    });
}

void journal_server::notice(std::string_view target, std::string_view message)
{
    cqueue_.push_back({
        { "command",    "notice"                },
        { "target",     std::string(target)     },
        { "message",    std::string(message)    }
    });
}

void journal_server::part(std::string_view channel, std::string_view reason)
{
    cqueue_.push_back({
        { "command",    "part"                  },
        { "channel",    std::string(channel)    },
        { "reason",     std::string(reason)     }
    });
}

void journal_server::send(std::string_view raw)
{
    cqueue_.push_back({
        { "command",    "send"                  },
        { "raw",        std::string(raw)        }
    });
}

void journal_server::topic(std::string_view channel, std::string_view topic)
{
    cqueue_.push_back({
        { "command",    "topic"                 },
        { "channel",    std::string(channel)    },
        { "topic",      std::string(topic)      }
    });
}

void journal_server::whois(std::string_view target)
{
    cqueue_.push_back({
        { "command",    "whois"                 },
        { "target",     std::string(target)     }
    });
}

} // !irccd

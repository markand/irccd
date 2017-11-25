/*
 * journal_server.cpp -- journaled server that logs every command
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

#include "journal_server.hpp"

namespace irccd {

void journal_server::reconnect() noexcept
{
    cqueue_.push_back({
        { "command",    "reconnect" }
    });
}

void journal_server::cmode(std::string channel, std::string mode)
{
    cqueue_.push_back({
        { "command",    "cmode"     },
        { "channel",    channel     },
        { "mode",       mode        }
    });
}

void journal_server::cnotice(std::string channel, std::string message)
{
    cqueue_.push_back({
        { "command",    "cnotice"   },
        { "channel",    channel     },
        { "message",    message     }
    });
}

void journal_server::invite(std::string target, std::string channel)
{
    cqueue_.push_back({
        { "command",    "invite"    },
        { "target",     target      },
        { "channel",    channel     }
    });
}

void journal_server::join(std::string channel, std::string password)
{
    cqueue_.push_back({
        { "command",    "join"      },
        { "channel",    channel     },
        { "password",   password    }
    });
}

void journal_server::kick(std::string target, std::string channel, std::string reason)
{
    cqueue_.push_back({
        { "command",    "kick"      },
        { "target",     target      },
        { "channel",    channel     },
        { "reason",     reason      }
    });
}

void journal_server::me(std::string target, std::string message)
{
    cqueue_.push_back({
        { "command",    "me"        },
        { "target",     target      },
        { "message",    message     }
    });
}

void journal_server::message(std::string target, std::string message)
{
    cqueue_.push_back({
        { "command",    "message"   },
        { "target",     target      },
        { "message",    message     }
    });
}

void journal_server::mode(std::string mode)
{
    cqueue_.push_back({
        { "command",    "mode"      },
        { "mode",       mode        }
    });
}

void journal_server::names(std::string channel)
{
    cqueue_.push_back({
        { "command",    "names"     },
        { "channel",    channel     }
    });
}

void journal_server::notice(std::string target, std::string message)
{
    cqueue_.push_back({
        { "command",    "notice"    },
        { "target",     target      },
        { "message",    message     }
    });
}

void journal_server::part(std::string channel, std::string reason)
{
    cqueue_.push_back({
        { "command",    "part"      },
        { "channel",    channel     },
        { "reason",     reason      }
    });
}

void journal_server::send(std::string raw)
{
    cqueue_.push_back({
        { "command",    "send"      },
        { "raw",        raw         }
    });
}

void journal_server::topic(std::string channel, std::string topic)
{
    cqueue_.push_back({
        { "command",    "topic"     },
        { "channel",    channel     },
        { "topic",      topic       }
    });
}

void journal_server::whois(std::string target)
{
    cqueue_.push_back({
        { "command",    "whois"     },
        { "target",     target      }
    });
}

} // !irccd

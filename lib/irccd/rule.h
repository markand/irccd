/*
 * rule.h -- rule filtering
 *
 * Copyright (c) 2013-2026 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_RULE_H
#define IRCCD_RULE_H

/**
 * \file rule.h
 * \brief Rule filtering.
 *
 * This module provides filtering support to accept or drop IRC events based
 * on several criteria.
 */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \brief Rule action
 */
enum irc_rule_action {
	/**
	 * Accept the event (default).
	 */
	IRC_RULE_ACCEPT,

	/**
	 * Drop the event.
	 */
	IRC_RULE_DROP
};

/**
 * \brief Describe a rule.
 *
 * A rule filter IRC server event before dispatching them to plugins.
 *
 * It can filter the following criteria:
 *
 * - servers (based on their names)
 * - channels
 * - origins (the entire origin)
 * - plugins (based on their names)
 * - events (in the form onMessage, onCommand, etc)
 *
 * Every criterion is implemented in a NULL-terminated list of values. If the
 * list itself is NULL the rule matches. If it's non-NULL, the rule will match
 * if the value is present within the list.
 */
struct irc_rule {
	/**
	 * (read-write)
	 *
	 * Accept or drop the event.
	 */
	enum irc_rule_action action;

	/**
	 * (read-only, optional)
	 *
	 * List of servers criteria.
	 */
	char **servers;

	/**
	 * (read-only, optional)
	 *
	 * List of channels criteria.
	 */
	char **channels;

	/**
	 * (read-only, optional)
	 *
	 * List of origins criteria.
	 */
	char **origins;

	/**
	 * (read-only, optional)
	 *
	 * List of plugins criteria.
	 */
	char **plugins;

	/**
	 * (read-only, optional)
	 *
	 * List of events criteria.
	 */
	char **events;

	/**
	 * \cond IRC_PRIVATE
	 */

	/**
	 * (private)
	 *
	 * Next rule in the linked list.
	 */
	struct irc_rule *next;

	/**
	 * (private)
	 *
	 * Previous rule in the linked list.
	 */
	struct irc_rule *prev;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

/**
 * Create a new empty rule that match everything with the given action.
 *
 * \param action the action
 * \return a new rule
 */
struct irc_rule *
irc_rule_new(enum irc_rule_action action);

/**
 * Add a new server in the rule criterion.
 *
 * \param rule the rule
 * \param value the value to add
 */
void
irc_rule_add_server(struct irc_rule *rule, const char *value);

/**
 * Remove a server from the rule criterion.
 *
 * \param rule the rule
 * \param value the value to remove
 */
void
irc_rule_remove_server(struct irc_rule *rule, const char *value);

/**
 * Add a new channel in the rule criterion.
 *
 * \param rule the rule
 * \param value the value to add
 */
void
irc_rule_add_channel(struct irc_rule *rule, const char *value);

/**
 * Remove a channel from the rule criterion.
 *
 * \param rule the rule
 * \param value the value to remove
 */
void
irc_rule_remove_channel(struct irc_rule *rule, const char *value);

/**
 * Add a new origin in the rule criterion.
 *
 * \param rule the rule
 * \param value the value to add
 */
void
irc_rule_add_origin(struct irc_rule *rule, const char *value);

/**
 * Remove a origin from the rule criterion.
 *
 * \param rule the rule
 * \param value the value to remove
 */
void
irc_rule_remove_origin(struct irc_rule *rule, const char *value);

/**
 * Add a new plugin in the rule criterion.
 *
 * \param rule the rule
 * \param value the value to add
 */
void
irc_rule_add_plugin(struct irc_rule *rule, const char *value);

/**
 * Remove a plugin from the rule criterion.
 *
 * \param rule the rule
 * \param value the value to remove
 */
void
irc_rule_remove_plugin(struct irc_rule *rule, const char *value);

/**
 * Add a new event in the rule criterion.
 *
 * \param rule the rule
 * \param value the value to add
 */
void
irc_rule_add_event(struct irc_rule *rule, const char *value);

/**
 * Remove a event from the rule criterion.
 *
 * \param rule the rule
 * \param value the value to remove
 */
void
irc_rule_remove_event(struct irc_rule *rule, const char *value);

/**
 * Check if the rule matches the given criteria provided as argument.
 *
 * Note: this only indicates if the rule is relevant to the given criterion, caller
 * must then check what to do with the rule itself (see ::rule::action).
 *
 * \param rule the rule to check
 * \param server the server name
 * \param channel the channel name
 * \param origin the origin
 * \param plugin the plugin name
 * \param event the event name (e.g. onMessage, onNotice)
 * \return non-zero if the rule matches
 */
int
irc_rule_match(const struct irc_rule *rule,
               const char *server,
               const char *channel,
               const char *origin,
               const char *plugin,
               const char *event);

/**
 * Identical to ::irc_rule_match except that this function will iterate through
 * the whole linked list provided in rule.
 */
int
irc_rule_matchlist(const struct irc_rule *rule,
                   const char *server,
                   const char *channel,
                   const char *origin,
                   const char *plugin,
                   const char *event);

/**
 * Free the rule.
 *
 * \param rule the rule
 */
void
irc_rule_free(struct irc_rule *rule);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_RULE_H */

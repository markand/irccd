/*
 * subst.c -- pattern substitution
 *
 * Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "subst.h"
#include "util.h"

struct pair {
	const char *key;
	const char *value;
};

struct attributes {
	char fg[16];
	char bg[16];
	char attrs[4][16];
	size_t attrsz;
};

static const struct pair irc_colors[] = {
	{ "white",      "0"     },
	{ "black",      "1"     },
	{ "blue",       "2"     },
	{ "green",      "3"     },
	{ "red",        "4"     },
	{ "brown",      "5"     },
	{ "purple",     "6"     },
	{ "orange",     "7"     },
	{ "yellow",     "8"     },
	{ "lightgreen", "9"     },
	{ "cyan",       "10"    },
	{ "lightcyan",  "11"    },
	{ "lightblue",  "12"    },
	{ "pink",       "13"    },
	{ "grey",       "14"    },
	{ "lightgrey",  "15"    },
	{ NULL,         NULL    }
};

static const struct pair irc_attrs[] = {
	{ "bold",       "\x02"  },
	{ "italic",     "\x09"  },
	{ "reverse",    "\x16"  },
	{ "strike",     "\x13"  },
	{ "underline",  "\x15"  },
	{ "underline2", "\x1f"  },
	{ NULL,         NULL    }
};

static const struct pair shell_fg[] = {
	{ "black",      "30"    },
	{ "red",        "31"    },
	{ "green",      "32"    },
	{ "orange",     "33"    },
	{ "blue",       "34"    },
	{ "purple",     "35"    },
	{ "cyan",       "36"    },
	{ "white",      "37"    },
	{ "default",    "39"    },
	{ NULL,         NULL    }
};

static const struct pair shell_bg[] = {
	{ "black",      "40"    },
	{ "red",        "41"    },
	{ "green",      "42"    },
	{ "orange",     "43"    },
	{ "blue",       "44"    },
	{ "purple",     "45"    },
	{ "cyan",       "46"    },
	{ "white",      "47"    },
	{ "default",    "49"    },
	{ NULL,         NULL    }
};

static const struct pair shell_attrs[] = {
	{ "bold",       "1"     },
	{ "dim",        "2"     },
	{ "underline",  "4"     },
	{ "blink",      "5"     },
	{ "reverse",    "7"     },
	{ "hidden",     "8"     },
	{ NULL,         NULL    }
};

static inline int
is_reserved(char token)
{
	return token == '#' || token == '@' || token == '$' || token == '!';
}

static inline int
scat(char **out, size_t *outsz, const char *value)
{
	size_t written;

	if ((written = irc_util_strlcpy(*out, value, *outsz)) >= *outsz)
		return -ENOMEM;

	*out += written;
	*outsz -= written;

	return 0;
}

static inline int
ccat(char **out, size_t *outsz, char c)
{
	if (*outsz == 0)
		return -ENOMEM;

	*(*out)++ = c;
	*(outsz) -= 1;

	return 0;
}

static void
attributes_parse(const char *key, struct attributes *attrs)
{
	char attributes[64] = {0}, *p;

	memset(attrs, 0, sizeof (*attrs));
	sscanf(key, "%15[^,],%15[^,],%63s", attrs->fg, attrs->bg, attributes);

	for (char *attr = attributes; *attr; ) {
		if ((p = strchr(attr, ',')))
			*p = 0;

		irc_util_strlcpy(attrs->attrs[attrs->attrsz++], attr, sizeof (attrs->attrs[0]));

		if (p)
			attr = p + 1;
		else
			*attr = '\0';
	}
}

static inline const char *
find(const struct pair *pairs, const char *key)
{
	for (const struct pair *pair = pairs; pair->key; ++pair)
		if (strcmp(pair->key, key) == 0)
			return pair->value;

	return NULL;
}

static int
subst_date(char *out, size_t outsz, const char *input, const struct irc_subst *subst)
{
	struct tm *tm = tm = localtime(&subst->time);

	if (strftime(out, outsz, input, tm) == 0)
		return -ENOMEM;

	return 0;
}

static int
subst_keyword(const char *key, char **out, size_t *outsz, const struct irc_subst *subst)
{
	const char *value = NULL;

	for (size_t i = 0; i < subst->keywordsz; ++i) {
		if (strcmp(subst->keywords[i].key, key) == 0) {
			value = subst->keywords[i].value;
			break;
		}
	}

	if (!value)
		return 0;

	return scat(out, outsz, value);
}

static int
subst_env(const char *key, char **out, size_t *outsz)
{
	const char *value;

	if (!(value = getenv(key)))
		return 0;

	return scat(out, outsz, value);
}

static void
subst_shell(const char *key, char **out, size_t *outsz)
{
	FILE *fp;
	size_t written;
	char *end;

	/* Accept silently. */
	if (!(fp = popen(key, "r")))
		return;

	/*
	 * Since we cannot determine the number of bytes that must be read,
	 * read until the end of the output string and cut at the number of
	 * bytes read if lesser.
	 */
	if ((written = fread(*out, 1, *outsz - 1, fp)) > 0) {
		/* Remove '\r\n' */
		if ((end = memchr(*out, '\r', written)) || (end = memchr(*out, '\n', written)))
			*end = '\0';
		else
			end = *out + written;

		*outsz -= end - *out;
		*out = end;
	}

	pclose(fp);
}

static int
subst_irc_attrs(const char *key, char **out, size_t *outsz)
{
	const char *value;
	struct attributes attrs;
	int rc;

	if (!key[0])
		return ccat(out, outsz, '\x03');

	attributes_parse(key, &attrs);

	/* At least a foreground or a font attribute is present. */
	if (attrs.fg[0] || attrs.attrs[0][0]) {
		if ((rc = ccat(out, outsz, '\x03')) < 0)
			return rc;

		/* Foreground. */
		if ((value = find(irc_colors, attrs.fg)) && (rc = scat(out, outsz, value)) < 0)
			return rc;

		/* Background. */
		if (attrs.bg[0]) {
			if ((rc = ccat(out, outsz, ',')) < 0)
				return rc;
			if ((value = find(irc_colors, attrs.bg)) && (rc = scat(out, outsz, value)) < 0)
				return rc;
		}

		/* Attributes. */
		for (size_t i = 0; i < attrs.attrsz; ++i)
			if ((value = find(irc_attrs, attrs.attrs[i])) && (rc = scat(out, outsz, value)) < 0)
				return rc;
	}

	return 0;
}

static int
subst_shell_attrs(char *key, char **out, size_t *outsz)
{
	const char *value;
	struct attributes attrs;
	int rc;

	/* Empty attributes means reset: @{}. */
	if (!key[0])
		return scat(out, outsz, "\033[0m");

	attributes_parse(key, &attrs);

	if ((rc = scat(out, outsz, "\033[")) < 0)
		return rc;

	/* Attributes first. */
	for (size_t i = 0; i < attrs.attrsz; ++i) {
		if ((value = find(shell_attrs, attrs.attrs[i])) && (rc = scat(out, outsz, value)) < 0)
			return rc;

		/* Need to append ; if we have still more attributes or colors next. */
		if ((i < attrs.attrsz || attrs.fg[0] || attrs.bg[0]) && (rc = ccat(out, outsz, ';')) < 0)
			return rc;
	}

	/* Foreground. */
	if (attrs.fg[0]) {
		if ((value = find(shell_fg, attrs.fg)) && (rc = scat(out, outsz, value)) < 0)
			return rc;
		if (attrs.bg[0] && (rc = ccat(out, outsz, ';')) < 0)
			return rc;
	}

	/* Background. */
	if (attrs.bg[0]) {
		if ((value = find(shell_bg, attrs.bg)) && (rc = scat(out, outsz, value)) < 0)
			return rc;
	}

	return ccat(out, outsz, 'm');
}

static int
subst_default(const char **p, char **out, size_t *outsz, const char *key)
{
	int rc;

	if ((rc = ccat(out, outsz, (*p)[-2])) < 0)
		return rc;
	if ((rc = ccat(out, outsz, '{')) < 0)
		return rc;
	if ((rc = scat(out, outsz, key)) < 0)
		return rc;
	if ((rc = ccat(out, outsz, '}')) < 0)
		return rc;

	return 0;
}

static int
substitute(const char **p, char **out, size_t *outsz, const struct irc_subst *subst)
{
	char key[64] = {}, *end;
	size_t keysz;
	int rc, replaced = 1;

	if (!**p)
		return 0;

	/* Find end of construction. */
	if (!(end = strchr(*p, '}')))
		return -EINVAL;

	/* Copy key. */
	if ((keysz = end - *p) >= sizeof (key))
		return -ENOMEM;

	memcpy(key, *p, keysz);

	switch ((*p)[-2]) {
	case '@':
		/* attributes */
		if (subst->flags & IRC_SUBST_IRC_ATTRS) {
			if ((rc = subst_irc_attrs(key, out, outsz)) < 0)
				return rc;
		} else if (subst->flags & IRC_SUBST_SHELL_ATTRS) {
			if ((rc = subst_shell_attrs(key, out, outsz)) < 0)
				return rc;
		} else
			replaced = 0;
		break;
	case '#':
		/* keyword */
		if (subst->flags & IRC_SUBST_KEYWORDS) {
			if ((rc = subst_keyword(key, out, outsz, subst)) < 0)
				return rc;
		} else
			replaced = 0;
		break;
	case '$':
		/* environment variable */
		if (subst->flags & IRC_SUBST_ENV) {
			if ((rc = subst_env(key, out, outsz)) < 0)
				return rc;
		} else
			replaced = 0;
		break;
	case '!':
		/* shell */
		if (subst->flags & IRC_SUBST_SHELL)
			subst_shell(key, out, outsz);
		else
			replaced = 0;
		break;
	default:
		break;
	}

	/* If substitution was disabled, put the token verbatim. */
	if (!replaced && (rc = subst_default(p, out, outsz, key)) < 0)
		return rc;

	/* Move after '}' */
	*p = end + 1;

	return 0;
}

ssize_t
irc_subst(char *out, size_t outsz, const char *in, const struct irc_subst *subst)
{
	assert(in);
	assert(out);
	assert(subst);

	char *o = out, *inalloc = NULL;
	ssize_t rc;

	if (!outsz)
		return 0;

	/*
	 * Always start with the date. To do that, we need to update the input
	 * to transform the strftime tokens. Then we use that new string as
	 * input. Since the final size must not exceed outsz, we use this as
	 * temporary buffer.
	 */
	if (subst->flags & IRC_SUBST_DATE) {
		inalloc = irc_util_calloc(1, outsz + 1);

		if ((rc = subst_date(inalloc, outsz, in, subst)) < 0)
			goto err;

		in = inalloc;
	}

	for (const char *i = in; *i && outsz; ) {
		/*
		 * Check if this is a reserved character, if it isn't go to the next character to
		 * see if it's valid otherwise we print it as last token.
		 *
		 * Example:
		 *   "#{abc}" -> keyword sequence
		 *   "abc #"  -> keyword sequence interrupted, kept as-is.
		 */
		if (!is_reserved(*i)) {
			if ((rc = ccat(&o, &outsz, *i++)) < 0)
				goto err;

			continue;
		}

		/*
		 * Test if after the reserved token we have the opening { construct. If it's the
		 * case we start substitution.
		 *
		 * Otherwise depending on what's after:
		 *   If it is the same reserved token, it is "escaped" and printed
		 *   If it is something else, we print the token and skip iteration.
		 *
		 * Examples:
		 *   ## => #
		 *   #@ => #@
		 *   ##{foo} => #{foo}
		 *   #{foo} => value
		 */
		if (*++i == '{') {
			/* Skip '{'. */
			++i;

			if ((rc = substitute(&i, &o, &outsz, subst)) < 0)
				goto err;
		} else {
			if (*i == i[-1])
				++i;
			if ((rc = ccat(&o, &outsz, i[-1])) < 0)
				goto err;
		}
	}

	if (outsz < 1) {
		rc = -ENOMEM;
		goto err;
	}

	free(inalloc);
	*o = '\0';

	return o - out;

err:
	free(inalloc);
	out[0] = '\0';

	return rc;
}

/*** pats.c -- pattern files
 *
 * Copyright (C) 2013-2015 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of glod.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include "pats.h"
#include "fops.h"
#include "nifty.h"

#if !defined warn
# define warn(x...)	errno = 0, error(x)
#endif	/* !warn */

typedef struct word_s word_t;
typedef struct wpat_s wpat_t;

struct word_s {
	size_t z;
	const char *s;
};

struct wpat_s {
	word_t w;
	word_t y;
	union {
		unsigned int u;
		struct {
			/* case insensitive? */
			unsigned int ci:1;
			/* whole word match or just prefix, suffix */
			unsigned int left:1;
			unsigned int right:1;
		};
	} fl/*ags*/;
};


#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

static void
__attribute__((format(printf, 1, 2), unused))
error(const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (errno) {
		fputs(": ", stderr);
		fputs(strerror(errno), stderr);
	}
	fputc('\n', stderr);
	return;
}


/* word level */
static word_t
snarf_word(const char *bp[static 1], const char *const ep)
{
	const char *wp;
	word_t res;

	/* find terminating double-quote " */
	for (wp = *bp; (wp = memchr(wp, '"', ep - wp)) != NULL; wp++) {
		if (LIKELY(wp[-1] != '\\')) {
			break;
		}
	}
	if (UNLIKELY(wp == NULL)) {
		warn("Error: no matching double-quote found");
		return (word_t){0UL};
	}
	/* check for unescaped newlines */
	with (const char *np = *bp) {
		if (UNLIKELY((np = memchr(np, '\n', wp - np)) != NULL) &&
		    UNLIKELY(np == *bp || np[-1] != '\\')) {
			warn("Error: unescaped newline");
			return (word_t){0UL};
		}
	}

	/* create the result */
	res = (word_t){
		.z = wp - *bp,
		.s = *bp,
	};
	/* advance BP */
	*bp = wp + 1U;
	return res;
}

static wpat_t
snarf_pat(word_t w)
{
	wpat_t res = {0U};

	/* check modifiers FIRST because we might change w.z */
	if (w.s[w.z + 1U] == 'i') {
		res.fl.ci = 1U;
	}
	/* check the word-boundary flags */
	if (UNLIKELY(w.s[0U] == '*')) {
		/* left boundary is a * */
		res.fl.left = 1U;
		w.s++;
		w.z--;
	}
	if (w.s[w.z - 1U] == '*' && w.s[w.z - 2U] != '\\') {
		/* right boundary is a * */
		res.fl.right = 1U;
		w.z--;
	}

	if (memchr(w.s, '\\', w.z) != NULL) {
		static char *word;
		static size_t worz;
		size_t ci = 0U;

		if (UNLIKELY(w.z + 1U > worz)) {
			worz = ((w.z + 1U) / 64U + 1U) * 64U;
			word = realloc(word, worz);
		}
		/* operate on static space for the fun of it */
		for (size_t wi = 0U; wi < w.z; wi++) {
			if (LIKELY((word[ci] = w.s[wi]) != '\\')) {
				/* increment */
				ci++;
			}
		}
		w = (word_t){.z = ci, .s = word};
	}
	res.w = w;
	return res;
}

static struct glod_pats_s*
append_pat(struct glod_pats_s *restrict g, wpat_t p)
{
	with (obint_t x = intern(g->oa_pat, p.w.s, p.w.z), y = 0U) {
		if (UNLIKELY(x / 64U > g->npats / 64U)) {
			/* extend */
			size_t nu = (x / 64U + 1U) * 64U * sizeof(*g->pats);

			g = realloc(g, sizeof(*g) + nu);
			if (UNLIKELY(g == NULL)) {
				break;
			}
			g->npats = x;
		} else if (x > g->npats) {
			g->npats = x;
		}
		if (p.y.s != NULL) {
			/* quickly get us a yield number */
			y = intern(g->oa_yld, p.y.s, p.y.z);
		}
		/* copy over pattern bits and bobs */
		g->pats[x - 1U] = (struct glod_pat_s){p.fl.u, p.w.z, 0U, y};
	}
	return g;
}


static glod_pats_t
__read_pats(const char *buf, size_t bsz)
{
	/* context, 0 for words, and 1 for yields */
	enum {
		CTX_W,
		CTX_Y,
	} ctx = CTX_W;
	struct glod_pats_s *res;
	wpat_t cur = {0U};
	obarray_t oa_pat;
	obarray_t oa_yld;

	/* get some resources on the way */
	with (const size_t iniz = 64U * sizeof(*res->pats)) {
		if (UNLIKELY((res = malloc(sizeof(*res) + iniz)) == NULL)) {
			return NULL;
		}
		res->npats = 0U;
		res->oa_pat = oa_pat = make_obarray();
		res->oa_yld = oa_yld = make_obarray();
	}

	/* now go through the buffer looking for " escapes */
	for (const char *bp = buf, *const ep = buf + bsz; bp < ep;) {
		switch (*bp++) {
		case '"': {
			/* we're inside a word */
			word_t w = snarf_word(&bp, ep);

			if (UNLIKELY(w.s == NULL)) {
				goto bugger;
			}

			/* append the word to cch for now */
			switch (ctx) {
			case CTX_W:
				cur = snarf_pat(w);
				break;
			case CTX_Y:
				cur.y = w;
				break;
			default:
				break;
			}
			break;
		}
		case 'A' ... 'Z':
		case 'a' ... 'z':
		case '0' ... '9':
		case '_':
			switch (ctx) {
			case CTX_Y:
				break;
			default:
				break;
			}
			break;
		case '-':
			/* could be -> (yield) */
			if (LIKELY(*bp == '>')) {
				/* yay, yield oper */
				ctx = CTX_Y;
				bp++;
			}
			break;
		case '\\':
			if (UNLIKELY(*bp == '\n')) {
				/* quoted newline, aka linebreak */
				bp++;
			}
			break;
		case '\n':
			/* switch back to W mode, and append current */
			if (LIKELY(cur.w.z > 0UL)) {
				res = append_pat(res, cur);
				if (UNLIKELY(res == NULL)) {
					goto bugger;
				}
			}
			ctx = CTX_W;
			cur = (wpat_t){0U};
			break;
		case '&':
		default:
			/* keep going */
			break;
		}
	}

	if (UNLIKELY(!res->npats)) {
		goto bugger;
	}
	/* materialise pattern strings */
	for (size_t i = 0U; i < res->npats; i++) {
		res->pats[i].p = obint_name(oa_pat, i + 1U);
		res->pats[i].idx = (unsigned int)i;
	}
	return res;

bugger:
	free_obarray(oa_pat);
	free_obarray(oa_yld);
	free(res);
	return NULL;
}


/* public API funs */
/**
 * Read patterns from file FN. */
glod_pats_t
glod_read_pats(const char *fn)
{
	glodfn_t f;
	glod_pats_t res = NULL;

	/* map the file FN and snarf the alerts */
	if (UNLIKELY((f = mmap_fn(fn, O_RDONLY)).fd < 0)) {
		goto out;
	} else if (UNLIKELY((res = __read_pats(f.fb.d, f.fb.z)) == NULL)) {
		goto out;
	}
	/* magic happens here */
	;

out:
	/* and out are we */
	(void)munmap_fn(f);
	return res;
}

void
glod_free_pats(glod_pats_t p)
{
	if (p->oa_pat != NULL) {
		free_obarray(p->oa_pat);
	}
	if (p->oa_yld != NULL) {
		free_obarray(p->oa_yld);
	}
	with (struct glod_pats_s *pp = deconst(p)) {
		free(pp);
	}
	return;
}

/* divide pats object for consumption with 2 backends */
glod_pats_t
glod_pats_filter(glod_pats_t pv, int(*f)(glod_pat_t))
{
	struct glod_pats_s *res;
	obarray_t oa_pat;
	obarray_t oa_yld;

	/* get some resources on the way */
	with (const size_t iniz = 64U * sizeof(*res->pats)) {
		if (UNLIKELY((res = malloc(sizeof(*res) + iniz)) == NULL)) {
			return NULL;
		}
		res->npats = 0U;
		oa_pat = make_obarray();
		oa_yld = pv->oa_yld;
	}

	for (size_t i = 0U; i < pv->npats; i++) {
		glod_pat_t p = pv->pats[i];

		if (!f(p)) {
			continue;
		}

		/* otherwise the filter predicate indicated success */
		with (obint_t x = intern(oa_pat, p.p, p.n)) {
			if (x == 0U) {
				/* oh god oh god */
				break;
			} else if (UNLIKELY(x / 64U > res->npats / 64U)) {
				/* extend */
				size_t nu = (x / 64U + 1U) * 64U * sizeof(p);

				res = realloc(res, sizeof(*res) + nu);
				if (UNLIKELY(res == NULL)) {
					goto bugger;
				}
				res->npats = x;
			} else if (x > res->npats) {
				res->npats = x;
			}
			/* just copy the whole shebang */
			res->pats[x - 1U] = p;
		}
	}

	if (UNLIKELY(!res->npats)) {
		goto bugger;
	}
	/* materialise pattern strings */
	for (size_t i = 0U; i < res->npats; i++) {
		res->pats[i].p = obint_name(oa_pat, i + 1U);
	}
	res->oa_pat = oa_pat;
	res->oa_yld = oa_yld;
	return res;

bugger:
	free_obarray(oa_pat);
	free(res);
	return NULL;
}

/* pats.c ends here */

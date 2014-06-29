/*** intern.c -- interning system
 *
 * Copyright (C) 2013-2014 Sebastian Freundt
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "intern.h"
#include "nifty.h"

/* a hash is the bucket locator and a chksum for collision detection */
typedef struct {
	size_t idx;
	uint_fast32_t chk;
} hash_t;

typedef struct {
	obint_t ob;
	uint_fast32_t ck;
} obcell_t;

struct obarray_s {
	/* next obint/number of obints */
	size_t obn;
	/* alloc size, 2-power */
	size_t obz;
	/* beef data */
	union {
		char c[];
		uint8_t u8[];
		uint32_t u32[];
		obcell_t oc[];
	} beef;
};

struct obarr_s {
	struct obarray_s *str;
	struct obarray_s *stk;
};

/* the beef table */
static struct obarr_s dflt;

static hash_t
murmur(const uint8_t *str, size_t len)
{
/* tokyocabinet's hasher */
	size_t idx = 19780211U;
	uint_fast32_t hash = 751U;
	const uint8_t *rp = str + len;

	while (len--) {
		idx = idx * 37U + *str++;
		hash = (hash * 31U) ^ *--rp;
	}
	return (hash_t){idx, hash};
}

static void*
recalloc(void *buf, size_t nmemb_ol, size_t nmemb_nu, size_t membz)
{
	nmemb_ol *= membz;
	nmemb_nu *= membz;
	buf = realloc(buf, nmemb_nu);
	memset((uint8_t*)buf + nmemb_ol, 0, nmemb_nu - nmemb_ol);
	return buf;
}

static obint_t
make_obint(obarray_t oa[static 1U], const char *str, size_t len)
{
/* put STR (of length LEN) into string obarray, don't check for dups */
#define OBAR_MINZ	(1024U)
#define obs		(*oa)->beef.u8
#define obn		(*oa)->obn
#define obz		(*oa)->obz
#define xtra		sizeof(*oa)
	/* make sure we pad with \0 bytes to the next 4-byte multiple */
	size_t pad = ((len / 4U) + 1U) * 4U;
	obint_t res;

	if (UNLIKELY(*oa == NULL)) {
		size_t nuz;

		for (nuz = OBAR_MINZ; pad >= nuz; nuz *= 2U);
		*oa = calloc(nuz + xtra, sizeof(*obs));
		obz = nuz;
	} else if (UNLIKELY(obn + pad >= obz)) {
		size_t nuz;

		for (nuz = (obz * 2U); obn + pad >= nuz; nuz *= 2U);
		*oa = recalloc(*oa, obz + xtra, nuz + xtra, sizeof(*obs));
		obz = nuz;
	}
	/* paste the string in question */
	memcpy(obs + (res = obn), str, len);
	/* assemble the result */
	res >>= 2U;
	res <<= 8U;
	res |= len;
	/* inc the obn pointer */
	obn += pad;
	return res;
#undef obs
#undef obn
#undef obz
#undef xtra
}

static inline size_t
obint_off(obint_t ob)
{
	/* mask out the length bit */
	return (ob >> 8U) << 2U;
}

static inline size_t
obint_len(obint_t ob)
{
	/* mask out the offset bit */
	return ob & 0b11111111U;
}


obint_t
intern(obarray_t UNUSED(oa), const char *str, size_t len)
{
#define SSTK_NSLOT	(256U)
#define SSTK_STACK	(4U * SSTK_NSLOT)
#define OBINT_MAX_LEN	(256U)
#define sstk		dflt.stk->beef.oc
#define nstk		dflt.stk->obn
#define zstk		dflt.stk->obz

	if (UNLIKELY(len == 0U || len >= OBINT_MAX_LEN)) {
		/* don't bother */
		return 0U;
	}

	/* we take 9 probes per 32bit value, hx.idx shifted by 3bits each
	 * then try the next stack
	 * the first stack is 256 entries wide, the next stack is 1024
	 * bytes wide, but only hosts 768 entries because the probe is
	 * constructed so that the lowest 8bits are always 0. */
	const hash_t hx = murmur((const uint8_t*)str, len);
	uint_fast32_t k = hx.idx;

	/* just try what we've got */
	if (UNLIKELY(!dflt.stk)) {
		size_t z = SSTK_STACK;
		dflt.stk = calloc(z + 2, sizeof(*sstk));
		zstk = z;
	}

	/* here's the initial probe then */
	for (size_t j = 0U; j < 9U; j++, k >>= 3U) {
		const size_t off = k & 0xffU;

		if (sstk[off].ck == hx.chk) {
			/* found him (or super-collision) */
			return sstk[off].ob;
		} else if (!sstk[off].ob) {
			/* found empty slot */
			obint_t ob = make_obint(&dflt.str, str, len);
			sstk[off].ob = ob;
			sstk[off].ck = hx.chk;
			nstk++;
			return ob;
		}
	}

	for (size_t i = SSTK_NSLOT, m = 0x3ffU;; i <<= 2U, m <<= 2U, m |= 3U) {
		/* reset k */
		k = hx.idx;

		if (UNLIKELY(i >= zstk)) {
			const size_t nu = i << 2U;
			dflt.stk = recalloc(dflt.stk, zstk, nu, sizeof(*sstk));
			zstk = nu;

			if (UNLIKELY(dflt.stk == NULL)) {
				zstk = 0UL, nstk = 0UL;
				break;
			}
		}

		/* here we probe within the top entries of the stack */
		for (size_t j = 0U; j < 9U; j++, k >>= 3U) {
			const size_t off = (i | k) & m;

			if (sstk[off].ck == hx.chk) {
				/* found him (or super-collision) */
				return sstk[off].ob;
			} else if (!sstk[off].ob) {
				/* found empty slot */
				obint_t ob = make_obint(&dflt.str, str, len);
				sstk[off].ob = ob;
				sstk[off].ck = hx.chk;
				nstk++;
				return ob;
			}
		}
	}
	return 0U;
#undef sstk
#undef nstk
#undef zstk
}

void
unintern(obarray_t UNUSED(oa), obint_t UNUSED(ob))
{
	return;
}

const char*
obint_name(obint_t ob)
{
	if (UNLIKELY(ob == 0UL)) {
		return NULL;
	} else if (UNLIKELY(dflt.str == NULL)) {
		return NULL;
	}
	return dflt.str->beef.c + obint_off(ob);
}

void
clear_interns(void)
{
	if (LIKELY(dflt.stk != NULL)) {
		free(dflt.stk);
	}
	dflt.stk = NULL;
	if (LIKELY(dflt.str != NULL)) {
		free(dflt.str);
	}
	dflt.str = NULL;
	return;
}

/* intern.c ends here */

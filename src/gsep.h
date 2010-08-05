/*** gsep.h -- guessing line oriented data formats
 *
 * Copyright (C) 2010 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <sebastian.freundt@ga-group.nl>
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

#if !defined INCLUDED_gsep_h_
#define INCLUDED_gsep_h_

#include <stdint.h>

#if !defined STATIC_GUTS
# define FDECL		extern
# define FDEFU
#else  /* STATIC_GUTS */
# define FDECL		static
# define FDEFU		static
#endif	/* !STATIC_GUTS */

/**
 * Supported delimiters. */
typedef enum {
	DLM_UNK,
	DLM_COMMA,
	DLM_SEMICOLON,
	DLM_TAB,
	DLM_PIPE,
	DLM_COLON,
	DLM_DOT,
	DLM_SPACE,
	DLM_NUL,
	NDLM
} dlm_t;

typedef struct gsep_ctx_s *gsep_ctx_t;

FDECL int gsep_in_line(char *line, size_t llen);
FDECL dlm_t gsep_assess(void);

FDECL void init_gsep(void);
FDECL void free_gsep(void);

/** return the number of occurrences of DLM per line. */
FDECL int gsep_get_sep_cnt(dlm_t dlm);
/** return the char representation of DLM. */
FDECL char gsep_get_sep_char(dlm_t dlm);

#endif	/* INCLUDED_gsep_h_ */

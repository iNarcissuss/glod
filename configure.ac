dnl -------------------------------------------------------------------------
dnl Autoconf startup.
dnl -------------------------------------------------------------------------

AC_PREREQ([2.63])
AC_INIT([glod], [0.1.7.GIT], [https://github.com/hroptatyr/glod])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl -------------------------------------------------------------------------
dnl Local copyright notices.
dnl -------------------------------------------------------------------------

AC_COPYRIGHT([dnl
#### Configuration script for glod
#### Copyright (C) 2010-2015  Sebastian Freundt

### Don't edit this script!
### This script was automatically generated by the `autoconf' program
### from the file `./configure.ac'.
### To rebuild it, execute the command
###     autoreconf
])

AM_INIT_AUTOMAKE([foreign parallel-tests dist-xz color-tests subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

## the build chain
AC_CANONICAL_HOST
AC_PROG_CC
SXE_CHECK_CC([c11 c1x c99 gnu99])
SXE_CHECK_CFLAGS([native=no])
AC_C_BIGENDIAN

## check for yuck helper
AX_CHECK_YUCK
AX_YUCK_SCMVER([version.mk])
AC_CONFIG_LINKS([GNUmakefile:GNUmakefile])

## check for clitoris helper
AX_CHECK_CLITORIS

## check for pseudo-tty support
AC_CHECK_HEADERS([pty.h])
AM_CONDITIONAL([HAVE_PTY_H], [test "${ac_cv_header_pty_h}" = "yes"])

## check for intrinsic support
AC_CHECK_HEADERS([mmintrin.h])
## check for intrinsics
SXE_CHECK_SIMD([_tzcnt_u32])
SXE_CHECK_SIMD([_tzcnt_u64])
SXE_CHECK_SIMD([_popcnt32])
SXE_CHECK_SIMD([_popcnt64])
SXE_CHECK_SIMD([_mm_popcnt_u32])
SXE_CHECK_SIMD([_mm_popcnt_u64])

SXE_CHECK_SIMD([_mm_add_epi8], [dnl
	__m128i x, y = _mm_setzero_si128();
	_mm_add_epi8(x, y)
])
SXE_CHECK_SIMD([_mm_cmpeq_epi8], [dnl
	__m128i x, y = _mm_setzero_si128();
	_mm_cmpeq_epi8(x, y)
])
SXE_CHECK_SIMD([_mm_movemask_epi8], [dnl
	__m128i x;
	_mm_movemask_epi8(x)
])
SXE_CHECK_SIMD([_mm_mpsadbw_epu8], [dnl
	__m128i x, y;
	_mm_mpsadbw_epu8(x, y, 0b101U)
])

SXE_CHECK_SIMD([_mm256_add_epi8], [dnl
	__m256i x, y = _mm256_setzero_si256();
	_mm256_add_epi8(x, y)
])
SXE_CHECK_SIMD([_mm256_cmpeq_epi8], [dnl
	__m256i x, y = _mm256_setzero_si256();
	_mm256_cmpeq_epi8(x, y)
])
SXE_CHECK_SIMD([_mm256_movemask_epi8], [dnl
	__m256i x;
	_mm256_movemask_epi8(x)
])
SXE_CHECK_SIMD([_mm256_mpsadbw_epu8], [dnl
	__m256i x, y;
	_mm256_mpsadbw_epu8(x, y, 0b101U)
])

SXE_CHECK_SIMD([_mm512_add_epi8], [dnl
	__m512i x, y = _mm512_setzero_si512();
	_mm512_add_epi8(x, y)
])
SXE_CHECK_SIMD([_mm512_cmp_epi8_mask], [dnl
	__m512i x, y = _mm512_setzero_si512();
	_mm512_cmp_epi8_mask(x, y, 0)
])
SXE_CHECK_SIMD([_mm512_cmpeq_epi8_mask], [dnl
	__m512i x, y = _mm512_setzero_si512();
	_mm512_cmpeq_epi8_mask(x, y)
])

if test "${ac_cv_func__mm_cmpeq_epi8}" = "yes" -a \
	"${ac_cv_func__mm_add_epi8}" = "yes"; then
	AC_DEFINE([HAVE_MM128_INT_INTRINS], [1], [define if _mm128_* integer intrinsics are usable])
fi
if test "${ac_cv_func__mm256_cmpeq_epi8}" = "yes" -a \
	"${ac_cv_func__mm256_add_epi8}" = "yes"; then
	AC_DEFINE([HAVE_MM256_INT_INTRINS], [1], [define if _mm256_* integer intrinsics are usable])
fi
if test "${ac_cv_func__mm512_cmp_epi8_mask}" = "yes" -a \
	"${ac_cv_func__mm512_add_epi8}" = "yes"; then
	AC_DEFINE([HAVE_MM512_INT_INTRINS], [1], [define if _mm512_* integer intrinsics are usable])
fi
if test "${ac_cv_func__mm_mpsadbw_epu8}" = "yes"; then
	AC_DEFINE([HAVE_MM128_SAD_INTRINS], [1], [define if sum-of-abs-diff intrinsics are usable])
fi
if test "${ac_cv_func__mm256_mpsadbw_epu8}" = "yes"; then
	AC_DEFINE([HAVE_MM256_SAD_INTRINS], [1], [define if sum-of-abs-diff intrinsics are usable])
fi

if test "${ac_cv_func__mm_popcnt_u32}" = "yes" -a \
	"${ac_cv_func__mm_popcnt_u64}" = "yes"; then
	AC_DEFINE([HAVE_POPCNT_INTRINS], [1], [define if popcnt intrinsics are usable])
fi

AM_CONDITIONAL([HAVE_INTRIN], [test "${ac_cv_type___m256i}" = "yes" -a "${ac_cv_func__tzcnt_u32}" = "yes"])

## record whether popcnt is there, one way or the other
AM_CONDITIONAL([HAVE_POPCNT], [test "${ac_cv_func__mm_popcnt_u32}" = "yes" -o "${ac_cv_func__mm_popcnt_u64}" = "yes" -o "${ac_cv_func__popcnt32}" = "yes" -o "${ac_cv_func__popcnt64}" = "yes"])

SXE_CHECK_CILK

## certain parts would only work on 64bit systems, check if we've got one
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if !defined _LP64
# error nope
#endif
]], [[]])], [sxe_cv_tgt_lp64="yes"], [sxe_cv_tgt_lp64="no"])
AM_CONDITIONAL([TGT_LP64], [test "${sxe_cv_tgt_lp64}" = "yes"])

AM_CONDITIONAL([HAVE_GLEP_REQS],
	[test 	"${ac_cv_type___m128i}" = "yes" -a \
		"${ac_cv_func__mm_add_epi8}" = "yes" -a \
		"${ac_cv_func__mm_cmpeq_epi8}" = "yes" -a \
		"${ac_cv_func__mm_movemask_epi8}" = "yes"])


## libtool goddess^Wgoodness
## has to be down here as we're turning -Werror'ing off
LT_INIT
SXE_CHECK_LIBTOOL

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([build-aux/Makefile])
AC_CONFIG_FILES([info/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT

echo
echo
echo "Build summary"
echo "============="
echo
echo "[[x]] enum"
echo "[[x]] gloom"
echo "[[x]] glid  (will not be installed)"
echo "[[x]] ldmatrix  (will not be installed)"
echo "[[x]] qgscore"
echo "[[x]] terms"
echo "[[x]] uncol"
echo "[[x]] unpgbrk"
echo "[[x]] unwrap"
if test "${HAVE_INTRIN_FALSE}" = "#"; then
	echo "[[x]] fastterms"
else
	echo "[[ ]] fastterms"
fi
if test "${HAVE_GLEP_REQS_FALSE}" = "#"; then
	echo "[[x]] glep"
else
	echo "[[ ]] glep"
fi
echo

## configure ends here
dnl configure.ac ends here

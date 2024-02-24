/* Copyright (C) 2012-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <immintrin.h>
#include <inttypes.h>
#include <string.h>
#include <libc-pointer-arith.h>
#include "str-two-way.h"

#ifndef FUNC_NAME
#  define FUNC_NAME __memmem_avx2
#endif
#ifndef VEC
#  define VEC __m256i
#endif
#ifndef MASK
#  define MASK uint32_t
#endif
#ifndef LOAD
#  define LOAD(x) _mm256_load_si256 (x)
#endif
#ifndef LOADU
#  define LOADU(x) _mm256_loadu_si256 (x)
#endif
#ifndef CMPEQ8_MASK
#  define CMPEQ8_MASK(x, y) _mm256_movemask_epi8 (_mm256_cmpeq_epi8 (x, y))
#endif
#ifndef SETONE8
#  define SETONE8(x) _mm256_set1_epi8 (x)
#endif
#ifndef TZCNT
#  define TZCNT(x) __builtin_ctz (x)
#endif
#ifndef BLSR
#  define BLSR(x) ((x) & ((x) -1))
#endif
#ifndef MEMMEM_GENERIC
#  define MEMMEM_GENERIC __memmem_generic
#endif
#ifndef TWO_WAY_LONG_NEEDLE_THRESHOLD
#  define TWO_WAY_LONG_NEEDLE_THRESHOLD VEC_SIZE
#endif
#ifndef VEC_SIZE
#  define VEC_SIZE 32
#endif
#define ONES ((MASK) -1)

#ifndef MEMCMPEQ
#  define MEMCMPEQ __memcmpeq
#endif
#ifndef MEMCPY
#  define MEMCPY memcpy
#endif
#ifndef MEMCHR
#  define MEMCHR memchr
#endif
#ifndef PAGE_SIZE
#  define PAGE_SIZE 4096
#endif
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define NOT_CROSSING_PAGE(p, obj_size)                                        \
  (PTR_DIFF (PTR_ALIGN_UP (p, PAGE_SIZE), p) >= obj_size)
#if TWO_WAY_LONG_NEEDLE_THRESHOLD > VEC_SIZE
#  define LONG_NEEDLE 1
#  define MIN_VEC(ne_len) MIN (ne_len, VEC_SIZE)
#else
#  define LONG_NEEDLE 0
#  define MIN_VEC(ne_len) (ne_len)
#endif

_Static_assert (VEC_SIZE == sizeof (VEC), "VEC_SIZE != sizeof (VEC).");
_Static_assert (
    TWO_WAY_LONG_NEEDLE_THRESHOLD <= VEC_SIZE * 2,
    "FIND_MATCH() assumes TWO_WAY_LONG_NEEDLE_THRESHOLD <= VEC_SIZE * 2.");

#if LONG_NEEDLE
#  define FIND_MATCH()                                                        \
    if (NOT_CROSSING_PAGE (hp, VEC_SIZE * 2))                                 \
      {                                                                       \
	/* Do a vector compare if we are not crossing a page. */              \
	hv = LOADU ((const VEC *) hp);                                        \
	cmpm = (MASK) CMPEQ8_MASK (hv, nv) << matchsh;                        \
	/* Compare only the relevant bits of the needle vector. */            \
	if (cmpm == matchm)                                                   \
	  {                                                                   \
	    if (ne_len <= VEC_SIZE)                                           \
	      return (void *) hp;                                             \
	    /* Compare the rest of the needle. */                             \
	    hv = LOADU ((const VEC *) hp + 1);                                \
	    cmpm = (MASK) CMPEQ8_MASK (hv, nv_e) << matchsh_e;                \
	    if (cmpm == matchm_e)                                             \
	      return (void *) hp;                                             \
	  }                                                                   \
      }                                                                       \
    else                                                                      \
      {                                                                       \
	if (!MEMCMPEQ (hp, ne, ne_len))                                       \
	  return (void *) hp;                                                 \
      }
#else
#  define FIND_MATCH()                                                        \
    if (NOT_CROSSING_PAGE (hp, VEC_SIZE))                                     \
      {                                                                       \
	hv = LOADU ((const VEC *) hp);                                        \
	cmpm = (MASK) CMPEQ8_MASK (hv, nv) << matchsh;                        \
	if (cmpm == matchm)                                                   \
	  return (void *) hp;                                                 \
      }                                                                       \
    else                                                                      \
      {                                                                       \
	if (!MEMCMPEQ (hp, ne, ne_len))                                       \
	  return (void *) hp;                                                 \
      }
#endif

extern void *MEMMEM_GENERIC (const void *, size_t, const void *,
			     size_t) attribute_hidden;

/* Lower is rarer. The table is based on the *.c and *.h files in glibc. */
extern const unsigned char ___rarebyte_table[256] attribute_hidden;

static inline void *__attribute__ ((always_inline))
find_rarest_byte (const unsigned char *rare, size_t n)
{
  const unsigned char *p = (const unsigned char *) rare;
  int c_rare = ___rarebyte_table[*rare];
  int c;
  for (; n--; ++p)
    {
      c = ___rarebyte_table[*p];
      if (c < c_rare)
	{
	  rare = p;
	  c_rare = c;
	}
    }
  return (void *) rare;
}

void *
FUNC_NAME (const void *hs, size_t hs_len, const void *ne, size_t ne_len)
{
  if (ne_len == 1)
    return (void *) MEMCHR (hs, *(unsigned char *) ne, hs_len);
  if (__glibc_unlikely (ne_len == 0))
    return (void *) hs;
  if (__glibc_unlikely (hs_len < ne_len))
    return NULL;
  /* Linear-time worst-case performance is guaranteed by the generic
   * implementation using the Two-Way algorithm. */
  if (__glibc_unlikely (ne_len > TWO_WAY_LONG_NEEDLE_THRESHOLD))
    return MEMMEM_GENERIC (hs, hs_len, ne, ne_len);
  VEC hv0, hv1, hv, nv;
  MASK i, hm0, hm1, m, cmpm;
  const unsigned int matchsh = ne_len < VEC_SIZE ? VEC_SIZE - ne_len : 0;
  const MASK matchm = ONES << matchsh;
#if LONG_NEEDLE
  VEC nv_e;
  const unsigned int matchsh_e
      = ne_len < VEC_SIZE * 2 ? VEC_SIZE * 2 - ne_len : 0;
  const MASK matchm_e = ONES << matchsh_e;
#endif
  const unsigned char *h = (const unsigned char *) hs;
  const unsigned char *const end = h + hs_len - ne_len;
  const unsigned char *hp;
  size_t rare = PTR_DIFF (
      find_rarest_byte ((const unsigned char *) ne, MIN_VEC (ne_len)), ne);
  /* RARE will always be the first byte to find.
     If RARE is at the end of the needle, use the byte before it. */
  if (rare == MIN_VEC (ne_len) - 1)
    --rare;
  const VEC nv0 = SETONE8 (*((char *) ne + rare));
  const VEC nv1 = SETONE8 (*((char *) ne + rare + 1));
  unsigned int off_e = (PTR_DIFF (end, h) < VEC_SIZE)
			   ? VEC_SIZE - (unsigned int) (end - h) - 1
			   : 0;
  /* Start from the position of RARE. */
  h += rare;
  /* Load the needle vector. */
  if (NOT_CROSSING_PAGE (ne, VEC_SIZE)
      || (LONG_NEEDLE ? ne_len >= VEC_SIZE : 0))
    nv = LOADU ((const VEC *) ne);
  else
    MEMCPY (&nv, ne, MIN_VEC (ne_len));
#if LONG_NEEDLE
  if (ne_len >= VEC_SIZE)
    {
      if (NOT_CROSSING_PAGE (ne, VEC_SIZE * 2))
	nv_e = LOADU ((const VEC *) ne + 1);
      else
	MEMCPY (&nv_e, (const unsigned char *) ne + VEC_SIZE,
		MIN (VEC_SIZE, ne_len - VEC_SIZE));
    }
#endif
  const unsigned int off_s = PTR_DIFF (h, PTR_ALIGN_DOWN (h, VEC_SIZE));
  /* Align down to VEC_SIZE. */
  h -= off_s;
  hv0 = LOAD ((const VEC *) h);
  hm0 = (MASK) CMPEQ8_MASK (hv0, nv0);
  hm1 = (MASK) CMPEQ8_MASK (hv0, nv1) >> 1;
  /* Clear the irrelevant bits from aligning down (OFF_S) and ones that are out
   * of bounds (OFF_E). */
  m = ((hm0 & hm1) >> off_s) & (ONES >> off_e);
  while (m)
    {
      i = TZCNT (m);
      m = BLSR (m);
      hp = h + off_s + i - rare;
      FIND_MATCH ();
    }
  h += VEC_SIZE - 1;
  for (; h - rare + VEC_SIZE <= end; h += VEC_SIZE)
    {
      hv0 = LOADU ((const VEC *) h);
      hv1 = LOAD ((const VEC *) (h + 1));
      hm1 = (MASK) CMPEQ8_MASK (hv1, nv1);
      hm0 = (MASK) CMPEQ8_MASK (hv0, nv0);
      m = hm0 & hm1;
      while (m)
	{
	match:
	  i = TZCNT (m);
	  m = BLSR (m);
	  hp = h + i - rare;
	  FIND_MATCH ();
	}
    }
  if (h - rare <= end)
    {
      off_e = VEC_SIZE - (unsigned int) (end - (h - rare)) - 1;
      hv0 = LOADU ((const VEC *) h);
      hv1 = LOAD ((const VEC *) (h + 1));
      hm1 = (MASK) CMPEQ8_MASK (hv1, nv1);
      hm0 = (MASK) CMPEQ8_MASK (hv0, nv0);
      /* Clear the irrelevant bits that are out of bounds. */
      m = hm0 & hm1 & (ONES >> off_e);
      if (m)
	goto match;
    }
  return NULL;
}

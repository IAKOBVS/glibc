/* Multiple versions of memmem.
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

/* Redefine memmem so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
#undef memmem
#define memmem __redirect_memmem
#include <string.h>
#undef memmem

#define MEMMEM __memmem_generic
#ifdef SHARED
#  undef libc_hidden_weak
#  define libc_hidden_weak(name)                                              \
    __hidden_ver1 (__memmem_generic, __GI_memmem, __memmem_generic);
#endif

#include "str-two-way.h"
#define TWO_WAY_LONG_NEEDLE_NON_STATIC
#include "string/memmem.c"

extern __typeof (__redirect_memmem) __memmem_avx2 attribute_hidden;
extern __typeof (__redirect_memmem) __memmem_avx512 attribute_hidden;
extern __typeof (__redirect_memmem) __memmem_generic attribute_hidden;
extern __typeof (__redirect_memmem) __memmem_sse2 attribute_hidden;

#define SYMBOL_NAME memmem

#include "init-arch.h"

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_memmem) __libc_memmem;

static inline void *
IFUNC_SELECTOR (void)
{
  const struct cpu_features *cpu_features = __get_cpu_features ();

  if (!CPU_FEATURES_ARCH_P (cpu_features, Prefer_No_AVX512)
      && CPU_FEATURE_USABLE_P (cpu_features, AVX512BW)
      && CPU_FEATURE_USABLE_P (cpu_features, BMI1))
    return __memmem_avx512;

  if (CPU_FEATURE_USABLE_P (cpu_features, AVX2)
      && CPU_FEATURE_USABLE_P (cpu_features, BMI1))
    return __memmem_avx2;

  if (CPU_FEATURES_ARCH_P (cpu_features, Fast_Unaligned_Load))
    return __memmem_sse2;

  return __memmem_generic;
}

libc_ifunc_redirected (__redirect_memmem, __libc_memmem, IFUNC_SELECTOR ());
#undef memmem
strong_alias (__libc_memmem, __memmem)

#include <x86intrin.h>

#define VEC __m128i
#define VEC_SIZE 16
#define MASK uint16_t
#define LOAD(x) _mm_load_si128 (x)
#define LOADU(x) _mm_loadu_si128 (x)
#define CMPEQ8_MASK(x, y) _mm_movemask_epi8 (_mm_cmpeq_epi8 (x, y))
#define SETONE8(x) _mm_set1_epi8 (x)
#define TZCNT(x)                                                              \
  ((x) ? _bit_scan_forward (x) : (MASK) sizeof (MASK) * CHAR_BIT)

#define FUNC_NAME __memmem_sse2
#define TWO_WAY_LONG_NEEDLE_THRESHOLD VEC_SIZE

#include "memmem-avx-base.h"

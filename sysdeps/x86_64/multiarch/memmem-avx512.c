#include "str-two-way.h"
#define MEMMEM_GENERIC TWO_WAY_LONG_NEEDLE_FUNC_NAME
#define TWO_WAY_LONG_NEEDLE_THRESHOLD ((VEC_SIZE) *2)
#define VEC_SIZE 64
#define VEC __m512i
#define MASK uint64_t
#define LOAD(x) _mm512_load_si512 (x)
#define LOADU(x) _mm512_loadu_si512 (x)
#define CMPEQ8_MASK(x, y) _mm512_cmpeq_epi8_mask (x, y)
#define SETONE8(x) _mm512_set1_epi8 (x)
#define TZCNT(x) _tzcnt_u64 (x)
#define FUNC_NAME __memmem_avx512
#include "memmem-avx-base.h"

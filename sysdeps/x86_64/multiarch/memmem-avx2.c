#include "str-two-way.h"
#define MEMMEM_GENERIC TWO_WAY_LONG_NEEDLE_FUNC_NAME
#define TWO_WAY_LONG_NEEDLE_THRESHOLD ((VEC_SIZE) *2)
#define VEC_SIZE 32
#define FUNC_NAME __memmem_avx2
#include "memmem-avx-base.h"

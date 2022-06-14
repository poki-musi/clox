#pragma once

#include "common.h"

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)
#define GROW_ARRAY(type, ptr, old_cap, new_cap)                                \
  (type *)reallocate(ptr, sizeof(type) * (old_cap), sizeof(type) * (new_cap))
#define FREE_ARRAY(type, ptr, old_cap)                                         \
  reallocate(ptr, sizeof(type) * (old_cap), 0)

void *reallocate(void *, size_t, size_t);

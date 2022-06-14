#pragma once

#include "common.h"

#ifdef DEBUG_TRACE_EXEC
#include "chunk.h"

void show_chunk(Chunk *, const char *);
int show_inst(Chunk *, int);
#endif

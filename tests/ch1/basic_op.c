#include <assert.h>
#include <string.h>

#define DEBUG_TRACE_EXEC

#include "chunk.h"
#include "debug.h"

uint8_t code[] = {
    OP_RET,
    OP_CONST,
    0,
};

int main()
{
  Chunk chunk;
  init_chunk(&chunk);

  write_byte(&chunk, OP_RET);
  write_constant(&chunk, 1.3);

  assert(memcmp(chunk.code, code, sizeof(code) / sizeof(uint8_t)) == 0);
  show_chunk(&chunk, "basic op");
  free_chunk(&chunk);
}

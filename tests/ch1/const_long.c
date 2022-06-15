#include <assert.h>
#include <string.h>

#define DEBUG_TRACE_EXEC

#include "chunk.h"
#include "debug.h"

uint8_t code[2 * 256 + 3];

int main()
{
  for(int i = 0; i < 256; i++)
  {
    code[i*2+0] = OP_CONST;
    code[i*2+1] = i;
  }
  code[2 * 256 + 0] = OP_CONST_LONG;
  code[2 * 256 + 1] = 0;
  code[2 * 256 + 2] = 1;

  Chunk chunk;
  init_chunk(&chunk);

  for(int i = 0; i < 257; i++)
    write_constant(&chunk, 1.3);

  assert(memcmp(chunk.code, code, sizeof(code) / sizeof(uint8_t)) == 0);
  show_chunk(&chunk, "const long");
  free_chunk(&chunk);
}

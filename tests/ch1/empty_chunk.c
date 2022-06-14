#include <assert.h>

#include "chunk.h"
#include "debug.h"

int main()
{
  Chunk chunk;
  init_chunk(&chunk);
  show_chunk(&chunk, "test chunk");
  free_chunk(&chunk);
}

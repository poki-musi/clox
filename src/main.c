#include "main.h"
#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char **argv)
{
  init_vm();

  Chunk chunk;
  init_chunk(&chunk);

  // TEST

  write_constant(&chunk, 1.3);
  for (int i = 0; i < 3; i++)
  {
    write_constant(&chunk, 1.3);
    write_byte(&chunk, OP_ADD);
  }

  // TEST

  show_chunk(&chunk, "test chunk");

  interpret(&chunk);
  free_vm();
  free_chunk(&chunk);
  return 0;
}

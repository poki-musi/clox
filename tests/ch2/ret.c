#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "debug.h"

int main()
{
  init_vm();

  Chunk chunk;
  init_chunk(&chunk);

  write_constant(&chunk, 1);
  write_byte(&chunk, OP_RET);

  interpret_chunk(&chunk);

  free_chunk(&chunk);
  free_vm();
}
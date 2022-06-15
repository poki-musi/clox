#include "chunk.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "vm.h"

#define BEGIN_TEST_CODE()                                                      \
  do                                                                           \
  {                                                                            \
    Chunk chunk;                                                               \
    init_chunk(&chunk)

#define END_TEST_CODE(res)                                                     \
  write_byte(&chunk, OP_RET);                                                  \
  interpret_chunk(&chunk);                                                     \
  Value pred = unsafe_prev_peek();                                             \
  printf("= ");                                                                \
  print_value(pred);                                                           \
  printf("\n");                                                                \
  free_chunk(&chunk);                                                          \
  }                                                                            \
  while (false)

/* assert(pred == (res)); \ */

int main()
{
  init_vm();

  BEGIN_TEST_CODE();
  {
    write_constant(&chunk, 1);
    write_constant(&chunk, 2);
    write_byte(&chunk, OP_MUL);
    write_constant(&chunk, 3);
    write_byte(&chunk, OP_ADD);
  }
  END_TEST_CODE(5);

  BEGIN_TEST_CODE();
  {
    write_constant(&chunk, 1);
    write_constant(&chunk, 2);
    write_constant(&chunk, 3);
    write_byte(&chunk, OP_MUL);
    write_byte(&chunk, OP_ADD);
  }
  END_TEST_CODE(7);

  BEGIN_TEST_CODE();
  {
    write_constant(&chunk, 3);
    write_constant(&chunk, 2);
    write_byte(&chunk, OP_SUB);
    write_constant(&chunk, 1);
    write_byte(&chunk, OP_SUB);
  }
  END_TEST_CODE(0);

  BEGIN_TEST_CODE();
  {
    write_constant(&chunk, 1);
    write_constant(&chunk, 2);
    write_constant(&chunk, 3);
    write_byte(&chunk, OP_MUL);
    write_byte(&chunk, OP_ADD);
    write_constant(&chunk, 4);
    write_constant(&chunk, 5);
    write_byte(&chunk, OP_NEG);
    write_byte(&chunk, OP_DIV);
    write_byte(&chunk, OP_SUB);
  }
  END_TEST_CODE(1 + 2 * 3 - 4.0 / -5.0);

  free_vm();
}

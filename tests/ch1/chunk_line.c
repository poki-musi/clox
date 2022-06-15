#include <assert.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_TRACE_EXEC

#include "chunk.h"
#include "debug.h"

const int lines[] = {4, 5, 5, 6, 6, 6, 9, 11};

const size_t lines_length = sizeof(lines) / sizeof(*lines);

int main()
{
  Chunk chunk;
  init_chunk(&chunk);

  write_nl(&chunk);
  write_nl(&chunk);
  write_nl(&chunk);
  write_byte(&chunk, OP_RET);
  write_nl(&chunk);
  write_byte(&chunk, OP_RET);
  write_byte(&chunk, OP_RET);
  write_nl(&chunk);
  write_byte(&chunk, OP_RET);
  write_byte(&chunk, OP_RET);
  write_byte(&chunk, OP_RET);
  write_nl(&chunk);
  write_nl(&chunk);
  write_nl(&chunk);
  write_byte(&chunk, OP_RET);
  write_nl(&chunk);
  write_nl(&chunk);
  write_byte(&chunk, OP_RET);

  int lines_test[lines_length];

  for(int i = 0; i < lines_length; i++)
  {
    lines_test[i] = chunk_line(&chunk, i);
    printf("test: %i - real: %i\n", lines_test[i], lines[i]);
  }

  assert(memcmp(lines, lines_test, lines_length) == 0);

  show_chunk(&chunk, "const long");
  free_chunk(&chunk);
}

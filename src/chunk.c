#include "value.h"
#include <stdlib.h>

#include "chunk.h"
#include "mem.h"

void init_chunk(Chunk *chunk)
{
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  init_valarr(&chunk->constants);
  chunk->lines = realloc(NULL, 1);
  *(chunk->lines) = 0;
  chunk->lines_size = 1;
}

void free_chunk(Chunk *chunk)
{
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  free(chunk->lines);
  free_valarr(&chunk->constants);
  init_chunk(chunk);
}

void add_nl(Chunk *chunk)
{
  if (chunk->lines[chunk->lines_size - 1] == 0)
  {
    chunk->lines[chunk->lines_size - 1]--;
  }
  else
  {
    size_t new_size = chunk->lines_size + 1;
    chunk->lines = realloc(chunk->lines, new_size);
    chunk->lines[new_size - 1] = 0;
    chunk->lines_size = new_size;
  }
}

int add_const(Chunk *chunk, Value value)
{
  write_valarr(&chunk->constants, value);
  return chunk->constants.count - 1;
}

int chunk_line(Chunk *chunk, const size_t offset)
{
  int line = 1;
  int line_count = 0;
  size_t line_ptr = 0;

  if (chunk->lines[0] < 0)
  {
    line += -chunk->lines[0];
    line_ptr++;
  }

  if (offset == 0)
    return line;

  for (size_t i = 0;;)
  {
    switch (chunk->code[i])
    {
    case OP_RET:
      i += 1;
      break;
    case OP_CONST:
      i += 2;
      break;
    }

    if (i >= offset)
      return line;

    if (line_count >= chunk->lines[line_ptr])
    {
      line_count = 0;
      line_ptr++;

      if (chunk->lines[line_ptr] < 0)
      {
        line += -chunk->lines[0];
        line_ptr++;
      }
    }
  }
}

//
// Writing Functions
//

void write_byte(Chunk *chunk, uint8_t byte)
{
  if (chunk->capacity < chunk->count + 1)
  {
    int old_cap = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(old_cap);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_cap, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->lines_size - 1]++;
  chunk->count++;
}

const int MASK_BYTE = (1 << 8) - 1;

void write_constant(Chunk *chunk, Value value)
{
  int ptr = add_const(chunk, value);
  if(ptr <= 255)
  {
    write_byte(chunk, OP_CONST);
    write_byte(chunk, ptr);
  }
  else
  {
    write_byte(chunk, OP_CONST_LONG);
    write_byte(chunk, ptr & MASK_BYTE);
    write_byte(chunk, ptr >> 8);
  }
}

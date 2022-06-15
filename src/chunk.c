#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "mem.h"
#include "value.h"

void init_chunk(Chunk *chunk)
{
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  init_valarr(&chunk->constants);
  chunk->lines = malloc(1 * sizeof(int));
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

int add_const(Chunk *chunk, Value value)
{
  write_valarr(&chunk->constants, value);
  return chunk->constants.count - 1;
}

//
// Line Number
//

inline void write_nl(Chunk *chunk)
{
  if (chunk->lines[chunk->lines_size - 1] <= 0)
  {
    chunk->lines[chunk->lines_size - 1]--;
  }
  else
  {
    size_t new_size = chunk->lines_size + 1;
    chunk->lines = realloc(chunk->lines, new_size * sizeof(int));
    chunk->lines[new_size - 1] = 0;
    chunk->lines_size = new_size;
  }
}

inline void write_nl_multiple(Chunk *chunk, unsigned int number)
{
  if (chunk->lines[chunk->lines_size - 1] <= 0)
  {
    chunk->lines[chunk->lines_size - 1] -= number;
  }
  else
  {
    size_t new_size = chunk->lines_size + 1;
    chunk->lines = realloc(chunk->lines, new_size * sizeof(int));
    chunk->lines[new_size - 1] = -number + 1;
    chunk->lines_size = new_size;
  }
}

inline int count_skip_nl(Chunk *chunk, int cur_line, size_t *line_ptr_ptr)
{
  int line_num = chunk->lines[*line_ptr_ptr];
  if (line_num <= 0)
  {
    cur_line += -line_num;
    (*line_ptr_ptr)++;
  }
  return cur_line;
}

int chunk_line(Chunk *chunk, const size_t offset)
{
  int line_count = 0;
  size_t line_ptr = 0;

  int line = count_skip_nl(chunk, 1, &line_ptr);

  for (size_t i = 0;;)
  {
    if (i >= offset)
      return line;

    int op = chunk->code[i];
    i += OFFSET_OP[op];
    line_count += OFFSET_OP[op];

    if (line_count >= chunk->lines[line_ptr])
    {
      line_count = 0;
      line_ptr++;
      line = count_skip_nl(chunk, line + 1, &line_ptr);
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
  chunk->count++;

  if (chunk->lines[chunk->lines_size - 1] >= 0)
  {
    chunk->lines[chunk->lines_size - 1]++;
  }
  else
  {
    size_t new_size = chunk->lines_size + 1;
    chunk->lines = realloc(chunk->lines, new_size * sizeof(int));
    chunk->lines[new_size - 1] = 1;
    chunk->lines_size = new_size;
  }
}

const int MASK_BYTE = (1 << 8) - 1;

void write_constant(Chunk *chunk, Value value)
{
  int ptr = add_const(chunk, value);
  if (ptr <= 255)
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

#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "value.h"

int simple_inst(const char *, int);
int const_inst(const char *, Chunk *, int);
int const_long_inst(const char *, Chunk *, int);

void show_chunk(Chunk *chunk, const char *name)
{
  printf("== %s ==\n", name);

  int line_count = 0;
  size_t line_ptr = 0;
  int cur_line = count_skip_nl(chunk, 1, &line_ptr);
  int prev_line = 0;

  for (int off = 0; off < chunk->count;)
  {
    printf("%04d ", off);

    if (prev_line != cur_line)
      printf("%04d ", cur_line);
    else
      printf("   | ");

    // Increment Bytecode Pointers
    int inc = show_inst(chunk, off);
    off += inc;
    line_count += inc;

    // Set current line count as the previous one
    prev_line = cur_line;

    // Increment current line count if at end of line
    // then count up any extra empty lines
    if (line_count >= chunk->lines[line_ptr])
    {
      line_count = 0;
      line_ptr++;
      cur_line = count_skip_nl(chunk, cur_line + 1, &line_ptr);
    }
  }
}

int show_inst(Chunk *chunk, int off)
{
  uint8_t inst = chunk->code[off];
  switch (inst)
  {
  case OP_RET:
    return simple_inst("OP_RET", off);
  case OP_CONST:
    return const_inst("OP_CONST", chunk, off);
  case OP_CONST_LONG:
    return const_long_inst("OP_CONST_LONG", chunk, off);
  case OP_NEG:
    return simple_inst("OP_NEG", off);
  case OP_ADD:
    return simple_inst("OP_ADD", off);
  case OP_SUB:
    return simple_inst("OP_SUB", off);
  case OP_MUL:
    return simple_inst("OP_MUL", off);
  case OP_DIV:
    return simple_inst("OP_DIV", off);
  default:
    printf("Unknown opcode %d\n", inst);
    return 1;
  }
}

inline int simple_inst(const char *name, int offset)
{
  printf("%s\n", name);
  return 1;
}

inline int const_inst(const char *name, Chunk *chunk, int offset)
{
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  print_value(chunk->constants.values[constant]);
  printf("'\n");
  return 2;
}

inline int const_long_inst(const char *name, Chunk *chunk, int offset)
{
  int constant = chunk->code[offset + 1] + (chunk->code[offset + 2] << 8);
  printf("%-16s %4d '", name, constant);
  print_value(chunk->constants.values[constant]);
  printf("'\n");
  return 3;
}

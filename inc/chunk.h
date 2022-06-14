#pragma once

#include "common.h"
#include "value.h"

typedef enum
{
  OP_RET,
  OP_CONST,
  OP_CONST_LONG,
  OP_NEG,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
} OpCode;

typedef struct
{
  int count;
  int capacity;
  uint8_t *code;
  ValueArray constants;
  int lines_size;
  int *lines;
} Chunk;

void init_chunk(Chunk *);
void free_chunk(Chunk *);
int add_const(Chunk *, Value);
void add_nl(Chunk *);
int chunk_line(Chunk *, size_t);

void write_byte(Chunk *, uint8_t);
void write_constant(Chunk *, Value);

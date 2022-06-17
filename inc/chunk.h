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
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_NOT,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
} OpCode;

static const int OFFSET_OP[] = {
    1, /* OP_RET, */
    2, /* OP_CONST, */
    3, /* OP_CONST_LONG, */
    1, /* OP_NEG, */
    1, /* OP_ADD, */
    1, /* OP_SUB, */
    1, /* OP_MUL, */
    1, /* OP_DIV, */
};

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

void write_nl(Chunk *);
void write_nl_multiple(Chunk *, unsigned int);
int count_skip_nl(Chunk *, int, size_t *);
int chunk_line(Chunk *, size_t);

void write_byte(Chunk *, uint8_t);
void write_constant(Chunk *, Value);

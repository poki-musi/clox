#pragma once

#include "common.h"
#include "value.h"
#include "chunk.h"

typedef struct {
  Chunk *chunk;
  uint8_t *ip;
  Value *stack;
  size_t stack_count;
  size_t stack_size;
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void init_vm();
void free_vm();

InterpretResult interpret(const char *);
InterpretResult interpret_chunk(Chunk *);
void push(Value);
Value pop();
Value peek(int);
Value unsafe_prev_peek();

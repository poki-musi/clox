#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "mem.h"
#include "vm.h"
#include "compiler.h"

VM vm;

static InterpretResult run();
static void reset_stack();

void init_vm()
{
  vm.stack = NULL;
  vm.stack_count = 0;
  vm.stack_size = 0;
  reset_stack();
}

static void reset_stack() { vm.stack_count = 0; }

void free_vm() { free(vm.stack); }

void push(Value value)
{
  if (vm.stack_size < vm.stack_count + 1)
  {
    int old_cap = vm.stack_size;
    vm.stack_size = GROW_CAPACITY(old_cap);
    vm.stack = GROW_ARRAY(Value, vm.stack, old_cap, vm.stack_size);
  }

  vm.stack[vm.stack_count] = value;
  vm.stack_count++;
}

#define VM_STACK_DEGROW 0.5
Value pop()
{
  Value ret = vm.stack[vm.stack_count - 1];
  vm.stack_count--;

  if (vm.stack_count < VM_STACK_DEGROW * vm.stack_size)
  {
    int old_cap = vm.stack_size;
    vm.stack_size = REDUCE_CAPACITY(old_cap);
    vm.stack = GROW_ARRAY(Value, vm.stack, old_cap, vm.stack_size);
  }

  return ret;
}

Value unsafe_prev_peek() { return vm.stack[vm.stack_count]; }

InterpretResult interpret(const char *source)
{
  Chunk chunk;
  init_chunk(&chunk);

  if(compile(source, &chunk)) {
    free_chunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  free_chunk(&chunk);
  return result;
}

InterpretResult interpret_chunk(Chunk *chunk)
{
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT(ptr) (vm.chunk->constants.values[ptr])
#define BINARY_OP(op)                                                          \
  do                                                                           \
  {                                                                            \
    Value second = pop();                                                      \
    Value first = pop();                                                       \
    push(first op second);                                                      \
  }                                                                            \
  while (false)

  for (;;)
  {

#ifdef DEBUG_TRACE_EXEC
    printf("          ");
    for (size_t i = 0; i < vm.stack_count; i++)
    {
      printf("[ ");
      print_value(vm.stack[i]);
      printf(" ]");
    }
    printf("\n");
    show_inst(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

    uint8_t inst;
    switch (inst = READ_BYTE())
    {
    case OP_RET:
      print_value(pop());
      printf("\n");
      return INTERPRET_OK;

    case OP_CONST:
      push(READ_CONSTANT(READ_BYTE()));
      break;

    case OP_CONST_LONG:
    {
      int ptr_1 = READ_BYTE();
      int ptr_2 = ((int)READ_BYTE()) << 8;
      push(READ_CONSTANT(ptr_1 + ptr_2));
    }
    break;

    case OP_NEG:
    {
      Value *top = &vm.stack[vm.stack_count - 1];
      *top = -*top;
    }
    break;

    case OP_ADD:
      BINARY_OP(+);
      break;

    case OP_SUB:
      BINARY_OP(-);
      break;

    case OP_MUL:
      BINARY_OP(*);
      break;

    case OP_DIV:
      BINARY_OP(/);
      break;
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_CONSTANT_LONG
}

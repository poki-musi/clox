#include "value.h"
#include <stdio.h>

#include "debug.h"
#include "vm.h"
#include <unistd.h>

VM vm;

static InterpretResult run();
static void reset_stack();

void init_vm() { reset_stack(); }

static void reset_stack() { vm.top = vm.stack; }

void free_vm() {}

void push(Value value)
{
  *vm.top = value;
  vm.top++;
}

Value pop()
{
  vm.top--;
  return *vm.top;
}

InterpretResult interpret(Chunk *chunk)
{
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG()                                                   \
  (vm.chunk->constants.values[READ_BYTE() + (READ_BYTE() << 8)])
#define BINARY_OP(op)                                                          \
  do                                                                           \
  {                                                                            \
    vm.top--;                                                                  \
    vm.top[-1] = vm.top[-1] op * vm.top;                                       \
  }                                                                            \
  while (false)

  for (;;)
  {
#ifdef DEBUG_TRACE_EXEC
    printf("          ");
    for (Value *slot = vm.stack; slot < vm.top; slot++)
    {
      printf("[ ");
      print_value(*slot);
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
      push(READ_CONSTANT());
      break;

    case OP_CONST_LONG:
      push(READ_CONSTANT_LONG());
      break;

    case OP_NEG:
    {
      Value *top = vm.top - 1;
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

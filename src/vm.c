#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "mem.h"
#include "value.h"
#include "vm.h"

VM vm;

static InterpretResult run();
static void reset_stack();

InterpretResult interpret(const char *source)
{
  Chunk chunk;
  init_chunk(&chunk);

  if (compile(source, &chunk))
  {
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

static void runtime_error(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t inst = vm.ip - vm.chunk->code - 1;
  int line = chunk_line(vm.chunk, inst);
  fprintf(stderr, "[line %d] in script\n", line);
  reset_stack();
}

void init_vm()
{
  vm.stack = NULL;
  vm.stack_count = 0;
  vm.stack_size = 0;
  reset_stack();
}

static void reset_stack() { vm.stack_count = 0; }

void free_vm() { free(vm.stack); }

//
// Stack Manip
//

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

Value peek(int off) { return vm.stack[vm.stack_count - off - 1]; }

Value *peek_ref(int off) { return &vm.stack[vm.stack_count - off - 1]; }

Value unsafe_prev_peek() { return vm.stack[vm.stack_count]; }

static bool is_falsey(Value value)
{
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static bool values_equal(Value a, Value b)
{
  if (a.type != b.type)
    return false;

  switch (a.type)
  {
  case VAL_BOOL:
    return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NUMBER:
    return AS_NUMBER(a) == AS_NUMBER(b);
  case VAL_NIL:
    return true;
  default:
    return false; // Unreachable
  }
}

//
//
//

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT(ptr) (vm.chunk->constants.values[ptr])
#define BINARY_OP(value_type, op)                                              \
  do                                                                           \
  {                                                                            \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))                            \
    {                                                                          \
      runtime_error("operands must be numbers");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double second = AS_NUMBER(pop());                                          \
    double first = AS_NUMBER(pop());                                           \
    push(value_type(first op second));                                         \
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

    case OP_NIL:
      push(NIL_VAL);
      break;

    case OP_FALSE:
      push(BOOL_VAL(false));
      break;

    case OP_TRUE:
      push(BOOL_VAL(true));
      break;

    case OP_EQUAL:
    {
      Value b = pop();
      Value a = pop();
      push(BOOL_VAL(values_equal(a, b)));
    }
    break;

    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;

    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;

    case OP_NEG:
    {
      if (!IS_NUMBER(peek(0)))
      {
        runtime_error("operand must be number");
        return INTERPRET_RUNTIME_ERROR;
      }
      Value *top = peek_ref(0);
      *top = NUMBER_VAL(-AS_NUMBER(*top));
    }
    break;

    case OP_NOT:
    {
      Value *top = peek_ref(0);
      *top = BOOL_VAL(is_falsey(*top));
    }
    break;

    case OP_ADD:
      BINARY_OP(NUMBER_VAL, +);
      break;

    case OP_SUB:
      BINARY_OP(NUMBER_VAL, -);
      break;

    case OP_MUL:
      BINARY_OP(NUMBER_VAL, *);
      break;

    case OP_DIV:
      BINARY_OP(NUMBER_VAL, /);
      break;
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_CONSTANT_LONG
}

#include <stdio.h>

#include "value.h"
#include "mem.h"

void init_valarr(ValueArray *arr)
{
  arr->values = NULL;
  arr->capacity = 0;
  arr->count = 0;
}

void write_valarr(ValueArray *arr, Value value)
{
  if (arr->capacity < arr->count + 1)
  {
    int oldCapacity = arr->capacity;
    arr->capacity = GROW_CAPACITY(oldCapacity);
    arr->values = GROW_ARRAY(Value, arr->values, oldCapacity, arr->capacity);
  }

  arr->values[arr->count] = value;
  arr->count++;
}

void free_valarr(ValueArray *arr)
{
  FREE_ARRAY(Value, arr->values, arr->capacity);
  init_valarr(arr);
}

void print_value(Value value)
{
  printf("%g", value);
}

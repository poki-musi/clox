#pragma once

#include "common.h"

typedef double Value;

typedef struct {
  int capacity;
  int count;
  Value *values;
} ValueArray;

void init_valarr(ValueArray *);
void write_valarr(ValueArray *, Value);
void free_valarr(ValueArray *);
void print_value(Value);

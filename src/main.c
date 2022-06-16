#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "vm.h"

static void repl();
static void run_file(const char *);
static char *read_file(const char *);

int main(int argc, const char **argv)
{
  init_vm();

  if (argc == 1)
  {
    repl();
  }
  else if (argc == 2)
  {
    run_file(argv[1]);
  }
  else
  {
    fprintf(stderr, "usage: clox [path]\n");
    exit(64);
  }

  free_vm();
  return 0;
}

static void repl()
{
  char line[1024];

  for (;;)
  {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin))
    {
      printf("\n");
      break;
    }

    interpret(line);
  }
}

static void run_file(const char *path)
{
  char *src = read_file(path);
  InterpretResult result = interpret(src);
  free(src);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

static char *read_file(const char *path)
{
  FILE *fh = fopen(path, "rb");
  if (fh == NULL)
  {
    fprintf(stderr, "could not open file \"%s\"\n", path);
    exit(74);
  }

  fseek(fh, 0L, SEEK_END);
  size_t fs = ftell(fh);
  rewind(fh);

  char *buffer = (char *)malloc(fs + 1);
  size_t bytes_read = fread(buffer, sizeof(char), fs, fh);
  if (bytes_read < fs)
  {
    fprintf(stderr, "could not read file \"%s\"\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(fh);
  return buffer;
}

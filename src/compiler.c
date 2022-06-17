#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "vm.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

//
// Parser
//

typedef struct
{
  Token current;
  Token previous;
  bool had_error;
  bool panic_mode;
} Parser;

Parser parser;
Chunk *compiling_chunk;

static inline Chunk *current_chunk();
static inline void error_at_current(const char *);
static inline void error(const char *);
static void error_at(Token *, const char *);

static void advance();
static void consume(TokenType, const char *);

static inline void emit_byte(uint8_t);
static inline void emit_bytes(uint8_t, uint8_t);
static inline void end_compiler();
static inline void emit_return();

//
// Rules
//

typedef enum
{
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct
{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

static void parse_precedence(Precedence);
static ParseRule *get_rule(TokenType);

static inline void expression();
static inline void binary();
static void literal();
static void number();
static void grouping();
static void unary();

ParseRule RULES[] = {
    [TKN_LEFT_PAREN] = {grouping, NULL, PREC_NONE},
    [TKN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TKN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TKN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TKN_COMMA] = {NULL, NULL, PREC_NONE},
    [TKN_DOT] = {NULL, NULL, PREC_NONE},
    [TKN_MINUS] = {unary, binary, PREC_TERM},
    [TKN_PLUS] = {NULL, binary, PREC_TERM},
    [TKN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TKN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TKN_STAR] = {NULL, binary, PREC_FACTOR},
    [TKN_BANG] = {unary, NULL, PREC_NONE},
    [TKN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TKN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TKN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TKN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TKN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TKN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TKN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TKN_IDENTIFIER] = {NULL, NULL, PREC_NONE},
    [TKN_STRING] = {NULL, NULL, PREC_NONE},
    [TKN_NUMBER] = {number, NULL, PREC_NONE},
    [TKN_AND] = {NULL, NULL, PREC_NONE},
    [TKN_CLASS] = {NULL, NULL, PREC_NONE},
    [TKN_ELSE] = {NULL, NULL, PREC_NONE},
    [TKN_FALSE] = {literal, NULL, PREC_NONE},
    [TKN_FOR] = {NULL, NULL, PREC_NONE},
    [TKN_FN] = {NULL, NULL, PREC_NONE},
    [TKN_IF] = {NULL, NULL, PREC_NONE},
    [TKN_NIL] = {literal, NULL, PREC_NONE},
    [TKN_OR] = {NULL, NULL, PREC_NONE},
    [TKN_PRINT] = {NULL, NULL, PREC_NONE},
    [TKN_RETURN] = {NULL, NULL, PREC_NONE},
    [TKN_SUPER] = {NULL, NULL, PREC_NONE},
    [TKN_THIS] = {NULL, NULL, PREC_NONE},
    [TKN_TRUE] = {literal, NULL, PREC_NONE},
    [TKN_VAR] = {NULL, NULL, PREC_NONE},
    [TKN_WHILE] = {NULL, NULL, PREC_NONE},
    [TKN_ERROR] = {NULL, NULL, PREC_NONE},
    [TKN_EOF] = {NULL, NULL, PREC_NONE},
};

//
// Main
//

int compile(const char *source, Chunk *chunk)
{
  init_scanner(source);
  parser.current.line = 1;
  compiling_chunk = chunk;

  parser.had_error = false;
  parser.panic_mode = false;

  advance();

  expression();

  consume(TKN_EOF, "expected end of expression");

  end_compiler();

  return parser.had_error;
}

//
// Parsing Utils
//

static void advance()
{
  parser.previous = parser.current;

  for (;;)
  {
    parser.current = scan_token();

    if (parser.current.type != TKN_ERROR)
      break;

    error_at_current(parser.current.start);
  }
}

static void consume(TokenType type, const char *msg)
{
  if (parser.current.type == type)
  {
    advance();
    return;
  }

  error_at_current(msg);
}

//
// Parsing
//

static void parse_precedence(Precedence precedence)
{
  advance();

  ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
  if (prefix_rule == NULL)
  {
    error("expected expression");
    return;
  }

  prefix_rule();

  while (precedence <= get_rule(parser.current.type)->precedence)
  {
    advance();
    ParseFn infix_rule = get_rule(parser.previous.type)->infix;
    infix_rule();
  }
}

static ParseRule *get_rule(TokenType type) { return &RULES[type]; }

static inline void expression() { parse_precedence(PREC_ASSIGNMENT); }

static void binary()
{
  TokenType op = parser.previous.type;
  ParseRule *rule = get_rule(op);
  parse_precedence((Precedence)(rule->precedence + 1));

  switch (op)
  {
  case TKN_PLUS:
    emit_byte(OP_ADD);
    break;
  case TKN_MINUS:
    emit_byte(OP_SUB);
    break;
  case TKN_STAR:
    emit_byte(OP_MUL);
    break;
  case TKN_SLASH:
    emit_byte(OP_DIV);
    break;

  case TKN_EQUAL_EQUAL:
    emit_byte(OP_EQUAL);
    break;

  case TKN_BANG_EQUAL:
    emit_byte(OP_EQUAL);
    emit_byte(OP_NEG);
    break;

  case TKN_GREATER:
    emit_byte(OP_GREATER);
    break;

  case TKN_GREATER_EQUAL:
    emit_byte(OP_LESS);
    emit_byte(OP_EQUAL);
    break;

  case TKN_LESS:
    emit_byte(OP_LESS);
    break;

  case TKN_LESS_EQUAL:
    emit_byte(OP_GREATER);
    emit_byte(OP_NEG);
    break;

  default:
    break; // Unreachable
  }
}

static void number()
{
  double value = strtod(parser.previous.start, NULL);
  if (current_chunk()->constants.count > UINT16_MAX)
  {
    error("too many constants in one chunk");
    return;
  }

  write_constant(current_chunk(), NUMBER_VAL(value));
}

static void literal()
{
  switch (parser.previous.type)
  {
  case TKN_FALSE:
    emit_byte(OP_FALSE);
    break;
  case TKN_TRUE:
    emit_byte(OP_TRUE);
    break;
  case TKN_NIL:
    emit_byte(OP_NIL);
    break;
  default:
    return; // Unreachable
  }
}

static void grouping()
{
  expression();
  consume(TKN_RIGHT_PAREN, "expect ')' after expression");
}

static void unary()
{
  TokenType op = parser.previous.type;

  parse_precedence(PREC_UNARY);

  switch (op)
  {
  case TKN_MINUS:
    emit_byte(OP_NEG);
    break;
  case TKN_BANG:
    emit_byte(OP_NOT);
    break;
  default:
    return; // Unreachable
  }
}

//
// Compiling
//

static inline void emit_byte(uint8_t byte)
{
  write_byte(current_chunk(), byte);
}

static inline void emit_bytes(uint8_t b1, uint8_t b2)
{
  emit_byte(b1);
  emit_byte(b2);
}

static inline void end_compiler()
{
  emit_return();
  show_chunk(current_chunk(), "code");
}

static inline void emit_return() { write_byte(current_chunk(), OP_RET); }

//
// Error
//

static inline Chunk *current_chunk() { return compiling_chunk; }

static inline void error_at_current(const char *msg)
{
  error_at(&parser.current, msg);
}

static inline void error(const char *message)
{
  error_at(&parser.previous, message);
}

static void error_at(Token *token, const char *msg)
{
  if (parser.panic_mode)
    return;

  parser.panic_mode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TKN_EOF)
  {
    fprintf(stderr, " at end");
  }
  else if (token->type == TKN_ERROR)
  {
    // Nothing
  }
  else
  {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", msg);
  parser.had_error = true;
}

//
// Other
//

int compile_test(const char *source)
{
  init_scanner(source);

  int line = 0;

  for (;;)
  {
    Token token = scan_token();
    if (token.line != line)
    {
      printf("%04d ", token.line);
      line = token.line;
    }
    else
    {
      printf("   | ");
    }
    printf("%2d '%.*s'\n", token.type, token.length, token.start);

    if (token.type == TKN_EOF)
      break;
  }

  return 0;
}

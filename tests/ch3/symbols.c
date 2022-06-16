#include <assert.h>

#include "scanner.h"

TokenType LIST[] = {
  // Single-character tokens.
  TKN_LEFT_PAREN,
  TKN_RIGHT_PAREN,
  TKN_LEFT_BRACE,
  TKN_RIGHT_BRACE,
  TKN_COMMA,
  TKN_DOT,
  TKN_MINUS,
  TKN_PLUS,
  TKN_SEMICOLON,
  TKN_SLASH,
  TKN_STAR,

  // One or two character tokens.
  TKN_BANG,
  TKN_BANG_EQUAL,
  TKN_EQUAL,
  TKN_EQUAL_EQUAL,
  TKN_GREATER,
  TKN_GREATER_EQUAL,
  TKN_LESS,
  TKN_LESS_EQUAL,

  // Keywords.
  TKN_AND,
  TKN_CLASS,
  TKN_ELSE,
  TKN_FALSE,
  TKN_FOR,
  TKN_FN,
  TKN_IF,
  TKN_NIL,
  TKN_OR,
  TKN_PRINT,
  TKN_RETURN,
  TKN_SUPER,
  TKN_THIS,
  TKN_TRUE,
  TKN_VAR,
  TKN_WHILE,
};

int main()
{
  init_scanner("( ) { } , . - + ; / *"
               "! != == = > >= < <="
               "and class else false for fn if nil or print return super this "
               "true var while");

  for (int i = 0; i < sizeof(LIST) / sizeof(*LIST); i++)
  {
    Token token = scan_token();
    if(token.type != LIST[i])
      exit(1);
  }
}

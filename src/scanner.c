#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct
{
  const char *start;
  const char *current;
  int line;
} Scanner;

Scanner scanner;

inline static bool at_end();
inline static char advance();
inline static bool match(char);
inline static char peek();
inline static char peek_next();
inline static bool is_digit(char);
inline static bool is_alphabetic(char);
inline static TokenType check_keyword(int, int, const char *, TokenType);

inline static void skip_whitespace();
inline static Token make_token(TokenType);
inline static Token error_token(const char *);
inline static Token make_string();
inline static Token make_number();
inline static Token make_id();
inline static TokenType identifier_type();

void init_scanner(const char *source)
{
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

Token scan_token()
{
  skip_whitespace();
  scanner.start = scanner.current;

  if (at_end())
    return make_token(TKN_EOF);

  char c = advance();

  if (is_digit(c))
    return make_number();

  if (is_alphabetic(c))
    return make_id();

  switch (c)
  {
  case '(':
    return make_token(TKN_LEFT_PAREN);
  case ')':
    return make_token(TKN_RIGHT_PAREN);
  case '{':
    return make_token(TKN_LEFT_BRACE);
  case '}':
    return make_token(TKN_RIGHT_BRACE);
  case ';':
    return make_token(TKN_SEMICOLON);
  case ',':
    return make_token(TKN_COMMA);
  case '/':
    return make_token(TKN_SLASH);
  case '.':
    return make_token(TKN_DOT);
  case '-':
    return make_token(TKN_MINUS);
  case '+':
    return make_token(TKN_PLUS);
  case '*':
    return make_token(TKN_STAR);
  case '!':
    return make_token(match('=') ? TKN_BANG_EQUAL : TKN_BANG);
  case '=':
    return make_token(match('=') ? TKN_EQUAL_EQUAL : TKN_EQUAL);
  case '<':
    return make_token(match('=') ? TKN_LESS_EQUAL : TKN_LESS);
  case '>':
    return make_token(match('=') ? TKN_GREATER_EQUAL : TKN_GREATER);
  case '"':
    return make_string();
  }

  return error_token("unexpected character");
}

inline static bool at_end() { return *scanner.current == '\0'; }

inline static char advance() { return *scanner.current++; }

inline static bool match(char expected)
{
  if (at_end() || *scanner.current != expected)
    return false;
  scanner.current++;
  return true;
}

inline static char peek() { return *scanner.current; }
inline static char peek_next()
{
  if (at_end())
    return '\0';
  else
    return scanner.current[1];
}

inline static bool is_digit(char c) { return '0' <= c && c <= '9'; }

inline static bool is_alphabetic(char c)
{
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

//
// Whitespace skipping
//

inline static void skip_whitespace()
{
  for (;;)
  {
    char c = peek();
    switch (c)
    {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      scanner.line++;
      advance();
      break;
    case '/':
      if (peek_next() == '/')
      {
        advance();
        advance();
        while (!at_end() && peek() != '\n')
          advance();
      }
      else
        return;
      break;
    default:
      return;
    }
  }
}

//
// Token Makers
//

inline static Token make_token(TokenType type)
{
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

inline static Token error_token(const char *msg)
{
  Token token;
  token.type = TKN_ERROR;
  token.start = msg;
  token.length = (int)strlen(msg);
  token.line = scanner.line;
  return token;
}

inline static Token make_string()
{
  while (peek() != '"' && !at_end())
  {
    if (peek() == '\n')
      scanner.line++;
    advance();
  }

  if (at_end())
    return error_token("unterminated string");

  advance();
  return make_token(TKN_STRING);
}

inline static Token make_number()
{
  while (is_digit(peek()))
    advance();

  if (peek() == '.' && is_digit(peek_next()))
  {
    advance();
    while (is_digit(peek()))
      advance();
  }

  return make_token(TKN_NUMBER);
}

inline static Token make_id()
{
  while (is_alphabetic(peek()) || is_digit(peek()))
    advance();
  return make_token(identifier_type());
}

inline static TokenType identifier_type()
{
  switch (scanner.start[0])
  {
  case 'a':
    return check_keyword(1, 2, "nd", TKN_AND);
  case 'c':
    return check_keyword(1, 4, "lass", TKN_CLASS);
  case 'e':
    return check_keyword(1, 3, "lse", TKN_ELSE);
  case 'i':
    return check_keyword(1, 1, "f", TKN_IF);
  case 'n':
    return check_keyword(1, 2, "il", TKN_NIL);
  case 'o':
    return check_keyword(1, 1, "r", TKN_OR);
  case 'p':
    return check_keyword(1, 4, "rint", TKN_PRINT);
  case 'r':
    return check_keyword(1, 5, "eturn", TKN_RETURN);
  case 's':
    return check_keyword(1, 4, "uper", TKN_SUPER);
  case 'v':
    return check_keyword(1, 2, "ar", TKN_VAR);
  case 'w':
    return check_keyword(1, 4, "hile", TKN_WHILE);

  case 'f':
    if (scanner.current - scanner.start > 1)
    {
      switch (scanner.start[1])
      {
      case 'a':
        return check_keyword(2, 3, "lse", TKN_FALSE);
      case 'o':
        return check_keyword(2, 1, "r", TKN_FOR);
      case 'n':
        return check_keyword(2, 0, "", TKN_FN);
      }
    }
    break;
  case 't':
    if (scanner.current - scanner.start > 1)
    {
      switch (scanner.start[1])
      {
      case 'h':
        return check_keyword(2, 2, "is", TKN_THIS);
      case 'r':
        return check_keyword(2, 2, "ue", TKN_TRUE);
      }
    }
    break;
  }

  return TKN_IDENTIFIER;
}

inline static TokenType check_keyword(int start, int length, const char *rest,
                                      TokenType type)
{
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0)
    return type;
  else
    return TKN_IDENTIFIER;
}

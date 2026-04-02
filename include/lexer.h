#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct {
  const char *src;
  int         pos;
  int         line;
  int         col;
} lexer_t;

lexer_t lexer_init(const char *src);

token_t next_token(lexer_t *l);

#endif

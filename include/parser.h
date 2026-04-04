#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct parser_t {
  lexer_t lexer;
  token_t current;
} parser_t;

parser_t parser_init(lexer_t lexer);
expr_t *parse(parser_t *parser);

#endif

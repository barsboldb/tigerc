#include "lexer.h"
#include "token.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

lexer_t lexer_init(const char *src) {
  return (lexer_t){
    .src  = src,
    .pos  = 0,
    .col  = 1,
    .line = 1,
  };
}

static char peek(lexer_t *lexer) {
  return lexer->src[lexer->pos];
}

static char advance(lexer_t *lexer) {
  char c = lexer->src[lexer->pos++];
  if (c == '\n') {
    lexer->line++;
    lexer->col = 1;
  } else {
    lexer->col++;
  }
  return c;
}

static int is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void skip_whitespace(lexer_t *lexer) {
  while (is_whitespace(peek(lexer))) {
    advance(lexer);
  }
}

static int skip_comment(lexer_t *lexer) {
  int depth = 1;
  while (depth > 0) {
    char c = advance(lexer);
    if (c == '\0') return -1;
    if (c == '*' && peek(lexer) == '/') {
      advance(lexer);
      depth--;
    } else if (c == '/' && peek(lexer) == '*') {
      advance(lexer);
      depth++;
    }
  }
  return 0;
}

static token_t lex_number(lexer_t *lexer, char first, int line, int col) {
  char buffer[256];
  int pos = 0;
  buffer[pos++] = first;

  while (isdigit(peek(lexer)) && pos < 254) {
    char c = advance(lexer);
    buffer[pos++] = c;
  }

  buffer[pos] = '\0';

  int val = atoi(buffer);

  return (token_t){ TOK_INT, .int_val = val, line, col };
}

static token_t lex_string(lexer_t *lexer, int line, int col) {
  char *buffer = malloc(sizeof(char) * 256);
  int pos = 0;

  while (1) {
    char c = advance(lexer);
    if (c == '\\') {
      char esc = advance(lexer);
      switch (esc) {
        case 'n':  buffer[pos++] = '\n'; break;
        case 't':  buffer[pos++] = '\t'; break;
        case '"':  buffer[pos++] = '"';  break;
        case '\\': buffer[pos++] = '\\'; break;
        default:   buffer[pos++] = esc;  break;
      }
    } else if (c == '"') {
      break;
    } else if (c == '\0') {
      free(buffer);
      return (token_t){ TOK_ERROR, .str_val = "unterminated string", line, col };
    } else {
      buffer[pos++] = c;
    }
  }

  buffer[pos] = '\0';
  
  return (token_t){ TOK_STRING, .str_val = buffer, line, col};
}

static token_t lex_id_or_keyword(lexer_t *lexer, char first, int line, int col) {
  char *buffer = malloc(256);
  int pos = 0;

  buffer[pos++] = first;

  while (1) {
    char c = peek(lexer);
    if (isalnum(c) || c == '_') {
      buffer[pos++] = advance(lexer);
      continue;
    }
    break;
  }

  buffer[pos] = '\0';

  if (strcmp(buffer, "while") == 0) {
    free(buffer);
    return (token_t){ TOK_WHILE, .int_val = 0, line, col };
  } else if (strcmp(buffer, "let") == 0) {
    free(buffer);
    return (token_t){ TOK_LET, .int_val = 0, line, col };
  } else if (strcmp(buffer, "var") == 0) {
    free(buffer);
    return (token_t){ TOK_VAR, .int_val = 0, line, col };
  } else if (strcmp(buffer, "then") == 0) {
    free(buffer);
    return (token_t){ TOK_THEN, .int_val = 0, line, col };
  } else if (strcmp(buffer, "nil") == 0) {
    free(buffer);
    return (token_t){ TOK_NIL, .int_val = 0, line, col };
  } else if (strcmp(buffer, "for") == 0) {
    free(buffer);
    return (token_t){ TOK_FOR, .int_val = 0, line, col };
  } else if (strcmp(buffer, "in") == 0) {
    free(buffer);
    return (token_t){ TOK_IN, .int_val = 0, line, col };
  } else if (strcmp(buffer, "type") == 0) {
    free(buffer);
    return (token_t){ TOK_TYPE, .int_val = 0, line, col };
  } else if (strcmp(buffer, "else") == 0) {
    free(buffer);
    return (token_t){ TOK_ELSE, .int_val = 0, line, col };
  } else if (strcmp(buffer, "to") == 0) {
    free(buffer);
    return (token_t){ TOK_TO, .int_val = 0, line, col };
  } else if (strcmp(buffer, "end") == 0) {
    free(buffer);
    return (token_t){ TOK_END, .int_val = 0, line, col };
  } else if (strcmp(buffer, "do") == 0) {
    free(buffer);
    return (token_t){ TOK_DO, .int_val = 0, line, col };
  } else if (strcmp(buffer, "break") == 0) {
    free(buffer);
    return (token_t){ TOK_BREAK, .int_val = 0, line, col };
  } else if (strcmp(buffer, "function") == 0) {
    free(buffer);
    return (token_t){ TOK_FUNCTION, .int_val = 0, line, col };
  } else if (strcmp(buffer, "if") == 0) {
    free(buffer);
    return (token_t){ TOK_IF, .int_val = 0, line, col };
  } else if (strcmp(buffer, "of") == 0) {
    free(buffer);
    return (token_t){ TOK_OF, .int_val = 0, line, col };
  } else {
    return (token_t){ TOK_ID, .str_val = buffer, line, col };
  }
}

token_t next_token(lexer_t *lexer) {
  skip_whitespace(lexer);

  int line = lexer->line;
  int col  = lexer->col;
  char c   = advance(lexer); 

  switch (c) {
    case '\0': return (token_t){ TOK_EOF, .int_val = 0, line, col };
    case '+': return (token_t){ TOK_PLUS, .int_val = 0, line, col };
    case '-': return (token_t){ TOK_MINUS, .int_val = 0, line, col };
    case '*': return (token_t){ TOK_STAR, .int_val = 0, line, col };
    case '/': 
      if (peek(lexer) == '*') {
        advance(lexer);
        if (skip_comment(lexer) < 0) {
          return (token_t){ TOK_ERROR, .str_val = "unterminated comment", line, col };
        };
        return next_token(lexer);
      }
      return (token_t){ TOK_SLASH, .int_val = 0, line, col };
    case '=': return (token_t){ TOK_EQ, .int_val = 0, line, col };
    case '&': return (token_t){ TOK_AND, .int_val = 0, line, col };
    case '|': return (token_t){ TOK_OR, .int_val = 0, line, col };
    case '(': return (token_t){ TOK_LPAREN, .int_val = 0, line, col };
    case ')': return (token_t){ TOK_RPAREN, .int_val = 0, line, col };
    case '[': return (token_t){ TOK_LBRACKET, .int_val = 0, line, col };
    case ']': return (token_t){ TOK_RBRACKET, .int_val = 0, line, col };
    case '{': return (token_t){ TOK_LBRACE, .int_val = 0, line, col };
    case '}': return (token_t){ TOK_RBRACE, .int_val = 0, line, col };
    case '.': return (token_t){ TOK_DOT, .int_val = 0, line, col };
    case ',': return (token_t){ TOK_COMMA, .int_val = 0, line, col };
    case ';': return (token_t){ TOK_SEMICOLON, .int_val = 0, line, col };
    case ':':
      if (peek(lexer) == '=') {
        advance(lexer);
        return (token_t){ TOK_ASSIGN, .int_val = 0, line, col };
      } else {
        return (token_t){ TOK_COLON, .int_val = 0, line, col };
      }
    case '<':
      if (peek(lexer) == '=') {
        advance(lexer);
        return (token_t){ TOK_LE, .int_val = 0, line, col };
      } else if (peek(lexer) == '>') {
        advance(lexer);
        return (token_t){ TOK_NEQ, .int_val = 0, line, col };
      } else {
        return (token_t){ TOK_LT, .int_val = 0, line, col };
      }
    case '>':
      if (peek(lexer) == '=') {
        advance(lexer);
        return (token_t){ TOK_GE, .int_val = 0, line, col };
      } else {
        return (token_t){ TOK_GT, .int_val = 0, line, col };
      }
    case '"': return lex_string(lexer, line, col);
    default:
      if (isdigit(c)) return lex_number(lexer, c, line, col);
      if (isalnum(c) || c == '_') return lex_id_or_keyword(lexer, c, line, col);
      return (token_t){ TOK_ERROR, .str_val = "unexpected character", line, col };
  }
}

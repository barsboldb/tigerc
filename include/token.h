#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
  // Literals
  TOK_INT,
  TOK_STRING,
  TOK_ID,

  // Keywords
  TOK_IF,
  TOK_THEN,
  TOK_ELSE,
  TOK_WHILE,
  TOK_FOR,
  TOK_TO,
  TOK_DO,
  TOK_LET,
  TOK_IN,
  TOK_OF,
  TOK_END,
  TOK_BREAK,
  TOK_NIL,
  TOK_FUNCTION,
  TOK_VAR,
  TOK_TYPE,

  // Operators
  TOK_PLUS,
  TOK_MINUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_EQ,
  TOK_NEQ,
  TOK_LT,
  TOK_GT,
  TOK_LE,
  TOK_GE,
  TOK_AND,
  TOK_OR,
  TOK_ASSIGN,

  // Punctuation
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_LBRACKET,
  TOK_RBRACKET,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_DOT,
  TOK_COMMA,
  TOK_SEMICOLON,
  TOK_COLON,

  // Special
  TOK_EOF,
  TOK_ERROR,
} token_type;

typedef struct {
  token_type type;
  union {
    int   int_val;
    char *str_val;
  };
  int line;
  int col;
} token_t;

const char *token_type_name(token_type type);

#endif

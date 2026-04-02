#include "token.h"

const char *token_type_name(token_type type) {
  switch (type) {
    case TOK_INT: return "TOK_INT";
    case TOK_STRING: return "TOK_STRING";
    case TOK_ID: return "TOK_ID";
    case TOK_IF: return "TOK_IF";
    case TOK_THEN: return "TOK_THEN";
    case TOK_ELSE: return "TOK_ELSE";
    case TOK_WHILE: return "TOK_WHILE";
    case TOK_FOR: return "TOK_FOR";
    case TOK_TO: return "TOK_TO";
    case TOK_DO: return "TOK_DO";
    case TOK_LET: return "TOK_LET";
    case TOK_IN: return "TOK_IN";
    case TOK_END: return "TOK_END";
    case TOK_BREAK: return "TOK_BREAK";
    case TOK_NIL: return "TOK_NIL";
    case TOK_FUNCTION: return "TOK_FUNCTION";
    case TOK_VAR: return "TOK_VAR";
    case TOK_TYPE: return "TOK_TYPE";
    case TOK_PLUS: return "TOK_PLUS";
    case TOK_MINUS: return "TOK_MINUS";
    case TOK_STAR: return "TOK_STAR";
    case TOK_SLASH: return "TOK_SLASH";
    case TOK_EQ: return "TOK_EQ";
    case TOK_NEQ: return "TOK_NEQ";
    case TOK_LT: return "TOK_LT";
    case TOK_GT: return "TOK_GT";
    case TOK_LE: return "TOK_LE";
    case TOK_GE: return "TOK_GE";
    case TOK_AND: return "TOK_AND";
    case TOK_OR: return "TOK_OR";
    case TOK_ASSIGN: return "TOK_ASSIGN";
    case TOK_LPAREN: return "TOK_LPAREN";
    case TOK_RPAREN: return "TOK_RPAREN";
    case TOK_LBRACKET: return "TOK_LBRACKET";
    case TOK_RBRACKET: return "TOK_RBRACKET";
    case TOK_LBRACE: return "TOK_LBRACE";
    case TOK_RBRACE: return "TOK_RBRACE";
    case TOK_DOT: return "TOK_DOT";
    case TOK_COMMA: return "TOK_COMMA";
    case TOK_SEMICOLON: return "TOK_SEMICOLON";
    case TOK_COLON: return "TOK_COLON";
    case TOK_EOF: return "TOK_EOF";
    case TOK_ERROR: return "TOK_ERROR";
    default: return "UNKNOWN";
  }
}

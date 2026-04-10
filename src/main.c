#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "printer.h"
#include "semant.h"
#include "symtab.h"
#include "types.h"

static symtab_t *base_tenv() {
  symtab_t *tenv = symtab_new(64);
  semty_t *int_ty    = malloc(sizeof(semty_t));
  semty_t *string_ty = malloc(sizeof(semty_t));
  int_ty->kind    = SEMTY_INT;
  string_ty->kind = SEMTY_STRING;
  symtab_enter_scope(tenv);
  symtab_insert(tenv, "int",    int_ty);
  symtab_insert(tenv, "string", string_ty);
  return tenv;
}

static symtab_t *base_venv() {
  symtab_t *venv = symtab_new(64);
  symtab_enter_scope(venv);
  return venv;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: tigerc <file>\n");
    return 1;
  }

  FILE *f = fopen(argv[1], "r");
  if (!f) {
    fprintf(stderr, "error: cannot open '%s'\n", argv[1]);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  rewind(f);

  char *src = malloc(len + 1);
  fread(src, 1, len, f);
  src[len] = '\0';
  fclose(f);

  lexer_t  lexer  = lexer_init(src);
  parser_t parser = parser_init(lexer);
  expr_t  *ast    = parse_expr(&parser);

  printf("=== AST ===\n");
  print_expr(ast, 0);

  printf("\n=== Semantic Analysis ===\n");
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  semty_t  *ty   = trans_expr(venv, tenv, ast);

  if (ty) {
    printf("type: ");
    switch (ty->kind) {
      case SEMTY_INT:    printf("int\n");    break;
      case SEMTY_STRING: printf("string\n"); break;
      case SEMTY_NIL:    printf("nil\n");    break;
      case SEMTY_VOID:   printf("void\n");   break;
      case SEMTY_RECORD: printf("record\n"); break;
      case SEMTY_ARRAY:  printf("array\n");  break;
      case SEMTY_NAME:   printf("name\n");   break;
    }
  } else {
    printf("type: (error or unknown)\n");
  }

  free(src);
  return 0;
}

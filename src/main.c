#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "printer.h"

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

  print_expr(ast, 0);

  free(src);
  return 0;
}

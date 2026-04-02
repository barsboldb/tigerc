#include <stdio.h>
#include "lexer.h"
#include "token.h"

int main(void) {
  const char *src = "let var x := 42 in x + 1 end";
  const char *src1 = "let var name := \"hello\" in if name <> \"world\" then 1 else 0 end";

  lexer_t lexer = lexer_init(src);
  lexer_t lexer1 = lexer_init(src1);

  token_t t;
  while ((t = next_token(&lexer)).type != TOK_EOF) {
    printf("%s\n", token_type_name(t.type));
  }
  printf("\n");

  while ((t = next_token(&lexer1)).type != TOK_EOF) {
    printf("%s\n", token_type_name(t.type));
  }
}

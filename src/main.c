#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "printer.h"
#include "semant.h"
#include "trans.h"
#include "frame.h"
#include "irprinter.h"
#include "frag.h"
#include "tree.h"
#include "symtab.h"
#include "types.h"
#include "canon.h"
#include "codegen.h"

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

  if (parser.error_count > 0) {
    fprintf(stderr, "aborting: %d parse error(s) found\n", parser.error_count);
    free(src);
    return 1;
  }

  printf("\n=== Semantic Analysis ===\n");
  symtab_t *tenv = semant_base_tenv();
  semant_base_venv(tenv);
  semty_t *ty = trans_expr(ast);

  if (semant_error_count() > 0) {
    fprintf(stderr, "aborting: %d type error(s) found\n", semant_error_count());
    return 1;
  }

  printf("type: ");
  switch (ty->kind) {
    case SEMTY_ERROR:  printf("error\n");  break;
    case SEMTY_INT:    printf("int\n");    break;
    case SEMTY_STRING: printf("string\n"); break;
    case SEMTY_NIL:    printf("nil\n");    break;
    case SEMTY_VOID:   printf("void\n");   break;
    case SEMTY_RECORD: printf("record\n"); break;
    case SEMTY_ARRAY:  printf("array\n");  break;
    case SEMTY_NAME:   printf("name\n");   break;
  }

  printf("\n=== Translation ===\n");
  symtab_t *aenv = symtab_new(64);
  symtab_enter_scope(aenv);
  frame_t  *top = frame_new("_main", NULL, 0, 0);
  tr_exp_t *ir  = tr_expr(aenv, top, ast);
  frag_insert(frag_proc(top, tree_move(tree_temp(frame_rv()), un_ex(ir))));
  symtab_exit_scope(aenv);
  print_frags();

  printf("\n=== Canonicalized IR ===\n");
  for (frag_t *fr = frag_list(); fr; fr = fr->next) {
    if (fr->kind != FRAG_PROC) continue;
    printf("PROC %s\n", fr->proc_.frame->name);
    stmt_list_t *stmts = trace_schedule(basic_blocks(linearize(fr->proc_.body)));
    for (stmt_list_t *sl = stmts; sl; sl = sl->next)
      print_tree_stmt(sl->stmt, 1);
  }

  printf("\n=== Abstract Assembly ===\n");
  for (frag_t *fr = frag_list(); fr; fr = fr->next) {
    if (fr->kind != FRAG_PROC) continue;
    printf("PROC %s\n", fr->proc_.frame->name);
    stmt_list_t *stmts = trace_schedule(basic_blocks(linearize(fr->proc_.body)));
    instr_t *instrs = codegen(fr->proc_.frame, stmts);
    print_instrs(instrs);
  }

  free(src);
  return 0;
}

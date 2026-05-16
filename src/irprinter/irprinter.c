#include <stdio.h>
#include "irprinter.h"
#include "frag.h"
#include "temp.h"

static void indent(int n) {
  for (int i = 0; i < n; i++) printf("  ");
}

static const char *relop_str(tree_relop_t op) {
  switch (op) {
    case TREE_EQ: return "=";
    case TREE_NE: return "<>";
    case TREE_LT: return "<";
    case TREE_GT: return ">";
    case TREE_LE: return "<=";
    case TREE_GE: return ">=";
  }
}

static const char *binop_str(tree_binop_t op) {
  switch (op) {
    case TREE_ADD: return "+";
    case TREE_SUB: return "-";
    case TREE_MUL: return "*";
    case TREE_DIV: return "/";
    case TREE_AND: return "&";
    case TREE_OR:  return "|";
    case TREE_XOR: return "^";
  }
}

void print_tree_expr(tree_expr_t *e, int d) {
  if (!e) { indent(d); printf("(null)\n"); return; }
  switch (e->kind) {
    case TREE_CONST:
      indent(d); printf("CONST %d\n", e->const_);
      break;
    case TREE_TEMP:
      indent(d); printf("TEMP t%d\n", e->temp);
      break;
    case TREE_NAME:
      indent(d); printf("NAME %s\n", label_name(e->name));
      break;
    case TREE_MEM:
      indent(d); printf("MEM\n");
      print_tree_expr(e->mem, d + 1);
      break;
    case TREE_BINOP:
      indent(d); printf("BINOP %s\n", binop_str(e->binop.op));
      print_tree_expr(e->binop.e1, d + 1);
      print_tree_expr(e->binop.e2, d + 1);
      break;
    case TREE_CALL:
      indent(d); printf("CALL\n");
      print_tree_expr(e->call.name, d + 1);
      for (int i = 0; i < e->call.num_actuals; i++)
        print_tree_expr(e->call.actuals[i], d + 1);
      break;
    case TREE_ESEQ:
      indent(d); printf("ESEQ\n");
      print_tree_stmt(e->eseq.s, d + 1);
      print_tree_expr(e->eseq.e, d + 1);
      break;
  }
}

void print_tree_stmt(tree_stmt_t *s, int d) {
  if (!s) { indent(d); printf("(null)\n"); return; }
  switch (s->kind) {
    case TREE_MOVE:
      indent(d); printf("MOVE\n");
      print_tree_expr(s->move.d, d + 1);
      print_tree_expr(s->move.s, d + 1);
      break;
    case TREE_EXP:
      indent(d); printf("EXP\n");
      print_tree_expr(s->exp, d + 1);
      break;
    case TREE_JUMP:
      indent(d); printf("JUMP %s\n", label_name(s->jump_.dests[0]));
      break;
    case TREE_CJUMP:
      indent(d); printf("CJUMP %s true=%s false=%s\n",
        relop_str(s->cjump.op),
        label_name(s->cjump.true_),
        label_name(s->cjump.false_));
      print_tree_expr(s->cjump.e1, d + 1);
      print_tree_expr(s->cjump.e2, d + 1);
      break;
    case TREE_SEQ:
      print_tree_stmt(s->seq.s1, d);
      print_tree_stmt(s->seq.s2, d);
      break;
    case TREE_LABEL:
      indent(d); printf("%s:\n", label_name(s->label));
      break;
  }
}

void print_frags(void) {
  for (frag_t *f = frag_list(); f; f = f->next) {
    if (f->kind == FRAG_STRING) {
      printf("STRING %s \"%s\"\n\n", label_name(f->string_.label), f->string_.str);
    } else {
      printf("PROC %s\n", f->proc_.frame->name);
      print_tree_stmt(f->proc_.body, 1);
      printf("\n");
    }
  }
}

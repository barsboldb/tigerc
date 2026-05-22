#include "canon.h"
#include "tree.h"
#include "temp.h"
#include <stdlib.h>

typedef struct {
  tree_stmt_t *stmts;
  tree_expr_t *expr;
} exp_res_t;

static exp_res_t *do_exp(tree_expr_t *e);

static tree_stmt_t *seq_stmts(tree_stmt_t *a, tree_stmt_t *b) {
  if (!a) return b;
  if (!b) return a;
  return tree_seq(a, b);
}

static stmt_list_t *linear(tree_stmt_t *s, stmt_list_t *rest) {
  if (s->kind != TREE_SEQ) {
    stmt_list_t *list = malloc(sizeof(stmt_list_t));
    list->stmt = s;
    list->next = rest;
    return list;
  }

  stmt_list_t *l1 = linear(s->seq.s2, rest);
  return linear(s->seq.s1, l1);
}

static tree_stmt_t *do_stm(tree_stmt_t *s) {
  switch (s->kind) {
    case TREE_SEQ:
      return tree_seq(do_stm(s->seq.s1), do_stm(s->seq.s2));
    case TREE_LABEL:
      return s;
    case TREE_JUMP: {
      exp_res_t *r = do_exp(s->jump_.target);
      return seq_stmts(r->stmts, tree_jump(r->expr, s->jump_.dests, s->jump_.num_dest));
    }
    case TREE_CJUMP: {
      exp_res_t *l = do_exp(s->cjump.e1);
      exp_res_t *r = do_exp(s->cjump.e2);
      tree_stmt_t *cj = tree_cjump(s->cjump.op, l->expr, r->expr, s->cjump.true_, s->cjump.false_);
      return seq_stmts(l->stmts, seq_stmts(r->stmts, cj));
    }
    case TREE_MOVE: {
      if (s->move.d->kind == TREE_MEM) {
        exp_res_t *addr = do_exp(s->move.d->mem);
        exp_res_t *src  = do_exp(s->move.s);
        tree_stmt_t *mv = tree_move(tree_mem(addr->expr), src->expr);
        return seq_stmts(addr->stmts, seq_stmts(src->stmts, mv));
      }
      exp_res_t *src = do_exp(s->move.s);
      return seq_stmts(src->stmts, tree_move(s->move.d, src->expr));
    }
    case TREE_EXP: {
      exp_res_t *r = do_exp(s->exp);
      return seq_stmts(r->stmts, tree_exp(r->expr));
    }
    default: return s;
  }
}

static exp_res_t *do_exp(tree_expr_t *e) {
  exp_res_t *r = malloc(sizeof(exp_res_t));
  switch (e->kind) {
    case TREE_CONST: 
    case TREE_TEMP:
    case TREE_NAME:
      r->stmts = NULL;
      r->expr  = e;
      return r;
    case TREE_BINOP: {
      exp_res_t *e1 = do_exp(e->binop.e1);
      exp_res_t *e2 = do_exp(e->binop.e2);
      r->stmts = seq_stmts(e1->stmts, e2->stmts);
      r->expr  = tree_binop(e->binop.op, e1->expr, e2->expr);
      return r;
    }
    case TREE_MEM: {
      exp_res_t *addr = do_exp(e->mem);
      r->stmts = addr->stmts;
      r->expr  = tree_mem(addr->expr);
      return r;
    }
    case TREE_ESEQ: {
      exp_res_t *res = do_exp(e->eseq.e);
      r->stmts = seq_stmts(do_stm(e->eseq.s), res->stmts);
      r->expr  = res->expr;
      return r;
    }
    case TREE_CALL: {
      temp_t t = temp_new();
      r->stmts = tree_move(tree_temp(t), e);
      r->expr  = tree_temp(t);
      return r;
    }
    default: r->stmts = NULL; r->expr = e; return r;
  }
}

stmt_list_t *linearize(tree_stmt_t *s) {
  return linear(do_stm(s), NULL);
}

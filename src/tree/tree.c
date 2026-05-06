#include <stdlib.h>
#include "tree.h"

tree_expr_t *tree_const(int val) {
  tree_expr_t *e = malloc(sizeof(tree_expr_t));
  e->kind = TREE_CONST;
  e->const_ = val;
  return e;
}
tree_expr_t *tree_temp(temp_t t) {
  tree_expr_t *e = malloc(sizeof(tree_expr_t));
  e->kind = TREE_TEMP;
  e->temp = t;
  return e;
}
tree_expr_t *tree_binop(tree_binop_t op, tree_expr_t *e1, tree_expr_t *e2) {
  tree_expr_t *e = malloc(sizeof(tree_expr_t));
  e->kind = TREE_BINOP;
  e->binop.op = op;
  e->binop.e1 = e1;
  e->binop.e2 = e2;
  return e;
}
tree_expr_t *tree_mem(tree_expr_t *ex) {
  tree_expr_t *e = malloc(sizeof(tree_expr_t));
  e->kind = TREE_MEM;
  e->mem  = ex;
  return e;
}
tree_expr_t *tree_call(tree_expr_t *name, tree_expr_t **actuals, int num_actuals) {
  tree_expr_t *e = malloc(sizeof(tree_expr_t));
  e->kind             = TREE_CALL;
  e->call.name        = name;
  e->call.actuals     = actuals;
  e->call.num_actuals = num_actuals;
  return e;
}
tree_expr_t *tree_eseq(tree_stmt_t *s, tree_expr_t *e) {
  tree_expr_t *ex = malloc(sizeof(tree_expr_t));
  ex->kind   = TREE_ESEQ;
  ex->eseq.s = s;
  ex->eseq.e = e;
  return ex;
}
tree_expr_t *tree_name(label_t l) {
  tree_expr_t *ex = malloc(sizeof(tree_expr_t));
  ex->kind = TREE_NAME;
  ex->name = l;
  return ex;
}

tree_stmt_t *tree_move(tree_expr_t *d, tree_expr_t *s) {
  tree_stmt_t *st = malloc(sizeof(tree_stmt_t));
  st->kind = TREE_MOVE;
  st->move.d = d;
  st->move.s = s;
  return st;
}
tree_stmt_t *tree_exp(tree_expr_t *e) {
  tree_stmt_t *st = malloc(sizeof(tree_stmt_t));
  st->kind = TREE_EXP;
  st->exp  = e;
  return st;
}
tree_stmt_t *tree_jump(tree_expr_t *target, label_t *dests, int num_dests) {
  tree_stmt_t *st = malloc(sizeof(tree_stmt_t));
  st->kind  = TREE_JUMP;
  st->jump_.target = target;
  st->jump_.dests  = dests;
  st->jump_.num_dest = num_dests;
  return st;
}
tree_stmt_t *tree_cjump(tree_relop_t op, tree_expr_t *e1, tree_expr_t *e2, label_t true_, label_t false_) {
  tree_stmt_t *st = malloc(sizeof(tree_stmt_t));
  st->kind = TREE_CJUMP;
  st->cjump.op = op;
  st->cjump.e1 = e1;
  st->cjump.e2 = e2;
  st->cjump.true_ = true_;
  st->cjump.false_ = false_;
  return st;
}
tree_stmt_t *tree_seq(tree_stmt_t *s1, tree_stmt_t *s2) {
  tree_stmt_t *st = malloc(sizeof(tree_stmt_t));
  st->kind = TREE_SEQ;
  st->seq.s1 = s1;
  st->seq.s2 = s2;
  return st;
}
tree_stmt_t *tree_label(label_t l) {
  tree_stmt_t *st = malloc(sizeof(tree_stmt_t));
  st->kind = TREE_LABEL;
  st->label = l;
  return st;
}

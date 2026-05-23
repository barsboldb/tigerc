#include "canon.h"
#include "tree.h"
#include "temp.h"
#include "symtab.h"
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

block_res_t *basic_blocks(stmt_list_t *stmts) {
  label_t done = label_new();
  if (!stmts || stmts->stmt->kind != TREE_LABEL) {
    stmt_list_t *sl = malloc(sizeof(stmt_list_t));
    sl->stmt = tree_label(label_new());
    sl->next = stmts;
    stmts = sl;
  }

  block_t *blocks = NULL, *blast = NULL;
  stmt_list_t *cur = stmts;

  while (cur) {
    stmt_list_t *block_head = cur;
    stmt_list_t *block_tail = cur;
    cur = cur->next;

    while (cur) {
      if (cur->stmt->kind == TREE_JUMP || cur->stmt->kind == TREE_CJUMP) {
        block_tail->next = cur;
        block_tail = cur;
        cur = cur->next;
        block_tail->next = NULL;
        break;
      } else if (cur->stmt->kind == TREE_LABEL) {
        stmt_list_t *jmp = malloc(sizeof(stmt_list_t));
        label_t *labels = malloc(sizeof(label_t));
        labels[0] = cur->stmt->label;
        jmp->stmt = tree_jump(tree_name(labels[0]), labels, 1);
        jmp->next = NULL;
        block_tail->next = jmp;
        block_tail = jmp;
        break;
      } else {
        block_tail = cur;
        cur = cur->next;
      }
    }

    if (!cur && block_tail->stmt->kind != TREE_JUMP && block_tail->stmt->kind != TREE_CJUMP) {
      stmt_list_t *jmp = malloc(sizeof(stmt_list_t));
      label_t *labels = malloc(sizeof(label_t));
      labels[0] = done;
      jmp->stmt = tree_jump(tree_name(done), labels, 1);
      jmp->next = NULL;
      block_tail->next = jmp;
      block_tail = jmp;
    }

    block_t *b = malloc(sizeof(block_t));
    b->stmts = block_head;
    b->next  = NULL;
    if (blast) blast = blast->next = b;
    else       blocks = blast = b;
  }

  block_res_t *res = malloc(sizeof(block_res_t));
  res->blocks = blocks;
  res->done   = done;
  return res;
}

stmt_list_t *trace_schedule(block_res_t *res) {
  symtab_t *tab = symtab_new(64);
  symtab_enter_scope(tab);
  for (block_t *b = res->blocks; b; b = b->next) {
    symtab_insert(tab, label_name(b->stmts->stmt->label), b);
  }

  stmt_list_t *result = NULL, *rlast = NULL;

  for (block_t *b = res->blocks; b; b = b->next) {
    if (symtab_lookup(tab, label_name(b->stmts->stmt->label)) == NULL)
      continue;

    block_t *trace = b;
    while (trace) {
      symtab_insert(tab, label_name(trace->stmts->stmt->label), NULL);

      stmt_list_t *tail = NULL;
      for (stmt_list_t *sl = trace->stmts; sl; sl = sl->next) {
        stmt_list_t *node = malloc(sizeof(stmt_list_t));
        node->stmt = sl->stmt;
        node->next = NULL;
        if (rlast) rlast = rlast->next = node;
        else       result = rlast = node;
        tail = node;
      }

      block_t *next = NULL;
      tree_stmt_t *last = tail->stmt;
      if (last->kind == TREE_JUMP) {
        next = symtab_lookup(tab, label_name(last->jump_.dests[0]));
      } else if (last->kind == TREE_CJUMP) {
        next = symtab_lookup(tab, label_name(last->cjump.false_));
        if (!next) next = symtab_lookup(tab, label_name(last->cjump.true_));
      }
      trace = next;
    }
  }

  stmt_list_t *donenode = malloc(sizeof(stmt_list_t));
  donenode->stmt = tree_label(res->done);
  donenode->next = NULL;
  if (rlast) rlast->next = donenode;
  else result = donenode;

  symtab_exit_scope(tab);
  return result;
}

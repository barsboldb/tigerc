#include <assert.h>
#include <stdlib.h>
#include "trans.h"
#include "frag.h"
#include "frame.h"
#include "tree.h"

static tree_binop_t map_binop(binop_t op) {
  switch (op) {
    case OP_ADD: return TREE_ADD;
    case OP_SUB: return TREE_SUB;
    case OP_MUL: return TREE_MUL;
    case OP_DIV: return TREE_DIV;
    default: assert(0);
  }
}
static tree_relop_t map_relop(binop_t op) {
  switch (op) {
    case OP_EQ: return TREE_EQ;
    case OP_NEQ: return TREE_NE;
    case OP_LT: return TREE_LT;
    case OP_LE: return TREE_LE;
    case OP_GT: return TREE_GT;
    case OP_GE: return TREE_GE;
    default: assert(0);
  }
}

static tr_exp_t *tr_ex(tree_expr_t *ex) {
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_EX;
  e->ex = ex;
  return e;
}

static tr_exp_t *tr_nx(tree_stmt_t *nx) {
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_NX;
  e->nx = nx;
  return e;
}

static tr_exp_t *tr_cx(cx_t cx) {
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_CX;
  e->cx   = cx;
  return e;
}

static tree_expr_t *frame_exp(access_t *a) {
  if (!a) return NULL;

  if (a->kind == ACCESS_REG) {
    return tree_temp(a->reg);
  } else {
    return tree_mem(tree_binop(TREE_ADD, tree_temp(frame_fp()), tree_const(a->offset)));
  }
}

tree_expr_t *un_ex(tr_exp_t *e) {
  switch (e->kind) {
    case TR_EX: {
      return e->ex;
    }
    case TR_NX: {
      return tree_eseq(e->nx, tree_const(0));
    }
    case TR_CX: {
      temp_t  t           = temp_new();
      label_t true_label  = label_new();
      label_t false_label = label_new();

      do_patch(e->cx.trues, true_label);
      do_patch(e->cx.falses, false_label);

      return tree_eseq(
        tree_seq(
          tree_move(tree_temp(t), tree_const(1)),
          tree_seq(
            e->cx.stm,
            tree_seq(
              tree_label(false_label),
              tree_seq(
                tree_move(tree_temp(t), tree_const(0)),
                tree_label(true_label)
              )
            )
          )
        ),
        tree_temp(t)
      );
    }
  }
}

tree_stmt_t *un_nx(tr_exp_t *e) {
  switch (e->kind) {
    case TR_EX: {
      return tree_exp(e->ex);
    }
    case TR_NX: {
      return e->nx;
    }
    case TR_CX: {
      label_t done = label_new();
      do_patch(e->cx.trues, done);
      do_patch(e->cx.falses, done);
      return tree_seq(e->cx.stm, tree_label(done));
    }
  }
}

cx_t un_cx(tr_exp_t *e) {
  switch (e->kind) {
    case TR_EX: {
      tree_stmt_t *stm = tree_cjump(TREE_NE, e->ex, tree_const(0), 0, 0);

      patch_list_t *trues = malloc(sizeof(patch_list_t));
      trues->head = &stm->cjump.true_;
      trues->next = NULL;

      patch_list_t *falses = malloc(sizeof(patch_list_t));
      falses->head = &stm->cjump.false_;
      falses->next = NULL;
      return (cx_t){ stm, trues, falses };
    }
    case TR_NX: {
      assert(0);
    }
    case TR_CX: {
      return e->cx;
    }
  }
}

void do_patch(patch_list_t *list, label_t label) {
  for (; list; list = list->next) {
    *(list->head) = label;
  }
}

patch_list_t *patch_list_new(label_ref_t l) {
  patch_list_t *pl = malloc(sizeof(patch_list_t));
  pl->head = l;
  pl->next = NULL;
  return pl;
}

patch_list_t *patch_list_join(patch_list_t *a, patch_list_t *b) {
  if (!a) return b;

  patch_list_t *cur = a;
  for (; cur->next; cur = cur->next);
  cur->next = b;
  return a;
}

static int cond_op(binop_t op) {
  switch (op) {
    case OP_EQ:
    case OP_NEQ:
    case OP_GE:
    case OP_GT:
    case OP_LE:
    case OP_LT:
      return 1;
    default: return 0;
  }
}

static tree_stmt_t *seq_append(tree_stmt_t *stmts, tree_stmt_t *s) {
  return stmts ? tree_seq(stmts, s) : s;
}

tr_exp_t *tr_expr(symtab_t *aenv, frame_t *frame, expr_t *e) {
  switch (e->kind) {
    case EXPR_INT:
      return tr_ex(tree_const(e->int_val));
    case EXPR_NIL:
      return tr_ex(tree_const(0));
    case EXPR_STRING: {
      label_t l = label_new();
      frag_insert(frag_str(l, e->str_val));
      return tr_ex(tree_name(l));
    }
    case EXPR_BINOP: {
      if (cond_op(e->binop.op)) {
        tree_stmt_t *stm = tree_cjump(
          map_relop(e->binop.op),
          un_ex(tr_expr(aenv, frame, e->binop.left)),
          un_ex(tr_expr(aenv, frame, e->binop.right)),
          0, 0
        );
        patch_list_t *trues = patch_list_new(&stm->cjump.true_);
        patch_list_t *falses = patch_list_new(&stm->cjump.false_);
        return tr_cx((cx_t){stm, trues, falses});
      }
      return tr_ex(
        tree_binop(
          map_binop(e->binop.op),
          un_ex(tr_expr(aenv, frame, e->binop.left)),
          un_ex(tr_expr(aenv, frame, e->binop.right))
        )
      );
    }
    case EXPR_CALL: {
      label_t l = label_named(e->call.id);
      expr_list_t *args = e->call.arg_list;
      int n;
      for (n = 0; args; args = args->next, n++);
      tree_expr_t **actuals = malloc(n * sizeof(tree_expr_t *));
      args = e->call.arg_list;
      for (int i = 0; i < n; i++, args = args->next) {
        actuals[i] = un_ex(tr_expr(aenv, frame, args->expr));
      }
      return tr_ex(tree_call(tree_name(l), actuals, n));
    }
    case EXPR_SEQ: {
      expr_list_t *list = e->seq;
      if (!list) return tr_ex(tree_const(0));
      if (!list->next) return tr_expr(aenv, frame, list->expr);

      tree_stmt_t *stmts = un_nx(tr_expr(aenv, frame, list->expr));
      list = list->next;
      while (list->next) {
        stmts = tree_seq(stmts, un_nx(tr_expr(aenv, frame, list->expr)));
        list = list->next;
      }

      return tr_ex(tree_eseq(stmts, un_ex(tr_expr(aenv, frame, list->expr))));
    }
    case EXPR_ID:
    case EXPR_INDEX:
    case EXPR_FIELD:
      return tr_var(aenv, frame, e);
    case EXPR_ASSIGN: {
      tr_exp_t *rhs = tr_expr(aenv, frame, e->assign.rhs);
      access_t *ac = symtab_lookup(aenv, e->assign.var);
      return tr_nx(tree_move(frame_exp(ac), un_ex(rhs)));
    }
    case EXPR_LET: {
      symtab_enter_scope(aenv);
      tree_stmt_t *stmts = NULL;
      dec_list_t  *declist = e->let.dec_list;
      while (declist) {
        stmts = seq_append(stmts, un_nx(tr_dec(aenv, frame, declist->dec)));
        declist = declist->next;
      }
      expr_list_t *body = e->let.body;
      while (body && body->next) {
        stmts = seq_append(stmts, un_nx(tr_expr(aenv, frame, body->expr)));
        body = body->next;
      }
      tree_expr_t *rv = !body ? tree_const(0) : un_ex(tr_expr(aenv, frame, body->expr));
      symtab_exit_scope(aenv);
      if (!stmts) return tr_ex(rv);

      return tr_ex(tree_eseq(stmts, rv));
    }
    case EXPR_IF: {
      if (!e->if_.else_) {
        label_t t_label = label_new();
        label_t done    = label_new();
        cx_t cond = un_cx(tr_expr(aenv, frame, e->if_.cond));
        do_patch(cond.trues, t_label);
        do_patch(cond.falses, done);
        return tr_nx(
          tree_seq(cond.stm,
            tree_seq(
              tree_label(t_label),
              tree_seq(
                un_nx(tr_expr(aenv, frame, e->if_.then)),
                tree_label(done)
              )
            )
          )
        );
      }
      label_t t_label = label_new();
      label_t f_label = label_new();
      label_t d_label = label_new();
      cx_t    cond    = un_cx(tr_expr(aenv, frame, e->if_.cond));
      temp_t  r       = temp_new();
      do_patch(cond.trues, t_label);
      do_patch(cond.falses, f_label);
      tree_stmt_t *then = tree_move(tree_temp(r), un_ex(tr_expr(aenv, frame, e->if_.then)));
      label_t *dests = malloc(sizeof(label_t));
      *dests = d_label;
      tree_stmt_t *done = tree_jump(tree_name(d_label), dests, 1);
      tree_stmt_t *else_ = tree_move(tree_temp(r), un_ex(tr_expr(aenv, frame, e->if_.else_)));
      return tr_ex(
        tree_eseq(
          tree_seq(
            cond.stm,
            tree_seq(
              tree_label(t_label),
              tree_seq(then,
                tree_seq(done,
                  tree_seq(tree_label(f_label),
                    tree_seq(else_,
                      tree_label(d_label)
                    )
                  )
                )
              )
            )
          ),
          tree_temp(r)
        )
      );
    }
    case EXPR_WHILE: {
      label_t t_label = label_new();
      label_t d_label = label_new();
      label_t b_label = label_new();
      cx_t cond = un_cx(tr_expr(aenv, frame, e->while_.cond));
      label_t *dests = malloc(sizeof(label_t));
      *dests = t_label;
      do_patch(cond.trues, b_label);
      do_patch(cond.falses, d_label);
      return tr_nx(
        tree_seq(
          tree_label(t_label),
          tree_seq(
            cond.stm,
            tree_seq(
              tree_label(b_label),
              tree_seq(
                un_nx(tr_expr(aenv, frame, e->while_.body)),
                tree_seq(
                  tree_jump(tree_name(t_label), dests, 1),
                  tree_label(d_label)
                )
              )
            )
          )
        )
      );
    }
    case EXPR_FOR: {
      access_t *acc = frame_alloc_local(frame, e->for_.escape);
      symtab_enter_scope(aenv);
      symtab_insert(aenv, e->for_.var, acc);
      label_t t_label   = label_new();
      label_t d_label   = label_new();
      label_t b_label   = label_new();
      tree_stmt_t *init = tree_move(frame_exp(acc), un_ex(tr_expr(aenv, frame, e->for_.init)));
      temp_t limit_temp = temp_new();
      tree_stmt_t *limit = tree_move(tree_temp(limit_temp), un_ex(tr_expr(aenv, frame, e->for_.to)));
      tree_stmt_t *body = un_nx(tr_expr(aenv, frame, e->for_.body));
      label_t *dests = malloc(sizeof(label_t));
      *dests = t_label;
      symtab_exit_scope(aenv);

      return tr_nx(
        tree_seq(
          init,
          tree_seq(
            limit,
            tree_seq(
              tree_label(t_label),
              tree_seq(
                tree_cjump(TREE_LE, frame_exp(acc), tree_temp(limit_temp), b_label, d_label),
                tree_seq(
                  tree_label(b_label),
                  tree_seq(
                    body,
                    tree_seq(
                      tree_move(
                        frame_exp(acc),
                        tree_binop(TREE_ADD, frame_exp(acc), tree_const(1))
                      ),
                      tree_seq(
                        tree_jump(tree_name(t_label), dests, 1),
                        tree_label(d_label)
                      )
                    )
                  )
                )
              )
            )
          )
        )
      );
    }
    case EXPR_RECORD: {
      // TODO: check named initialization. Current one is positional initialization
      int n = 0;
      for (field_list_t *p = e->record.fields; p; p = p->next, n++);
      temp_t r = temp_new();
      tree_expr_t **alloc_actual = malloc(sizeof(tree_expr_t *));
      *alloc_actual = tree_const(n * WORD_SIZE);
      tree_stmt_t *alloc = tree_move(
        tree_temp(r),
        tree_call(
          tree_name(label_named("malloc")), alloc_actual, 1
        )
      );
      field_list_t *fields = e->record.fields;
      tree_stmt_t *field_inits = NULL;
      for (int i = 0; i < n; i++, fields = fields->next) {
        tree_stmt_t *instr = tree_move(
          tree_mem(tree_binop(TREE_ADD, tree_temp(r), tree_const(i * WORD_SIZE))),
          un_ex(tr_expr(aenv, frame, fields->val))
        );
        field_inits = seq_append(field_inits, instr);
      }
      tree_stmt_t *all = field_inits ? tree_seq(alloc, field_inits) : alloc;
      return tr_ex(tree_eseq(all, tree_temp(r)));
    }
    case EXPR_ARRAY: {
      tree_expr_t **actuals = malloc(2 * sizeof(tree_expr_t *));
      actuals[0] = un_ex(tr_expr(aenv, frame, e->array.size));
      actuals[1] = un_ex(tr_expr(aenv, frame, e->array.init));
      return tr_ex(
        tree_call(tree_name(label_named("init_array")), actuals, 2)
      );
    }
  }
}
tr_exp_t *tr_dec(symtab_t *aenv, frame_t *frame, dec_t *d) {
  switch (d->kind) {
    case DEC_TYPE: 
      return tr_nx(tree_exp(tree_const(0)));

    case DEC_VAR: {
      access_t *access = frame_alloc_local(frame, d->var.escape);
      tr_exp_t *init   = tr_expr(aenv, frame, d->var.init);
      symtab_insert(aenv, d->var.id, access);
      return tr_nx(
        tree_move(
          frame_exp(access),
          un_ex(init)
        )
      );
    }
    case DEC_FUNC: {
      int n = 0;
      param_list_t *p = d->func.args;
      for (; p; p = p->next, n++);
      int *escapes = malloc(n * sizeof(int));
      p = d->func.args;
      for (int i = 0; i < n; p = p->next, i++) {
        escapes[i] = p->param->escape;
      }
      frame_t *f = frame_new(d->func.id, escapes, n);
      free(escapes);
      symtab_enter_scope(aenv);
      p = d->func.args;
      for (int i = 0; i < n; p = p->next, i++) {
        symtab_insert(aenv, p->param->name, &f->formals[i]);
      }
      tr_exp_t *body = tr_expr(aenv, f, d->func.body);

      frag_insert(frag_proc(f, tree_move(tree_temp(frame_rv()), un_ex(body))));

      symtab_exit_scope(aenv);
      return tr_nx(tree_exp(tree_const(0)));
    }
  }
}
tr_exp_t *tr_var(symtab_t *aenv, frame_t *frame, expr_t *e) {
  switch (e->kind) {
    case EXPR_ID: {
      access_t *a = symtab_lookup(aenv, e->id);
      return tr_ex(frame_exp(a));
    }
    case EXPR_INDEX: {
      tree_expr_t *base = un_ex(tr_var(aenv, frame, e->index_.array));
      tree_expr_t *index = un_ex(tr_expr(aenv, frame, e->index_.index));
      return tr_ex(
        tree_mem(
          tree_binop(
            TREE_ADD,
            base, 
            tree_binop(
              TREE_MUL, 
              index, 
              tree_const(WORD_SIZE)
            )
          )
        )
      );
    }
    case EXPR_FIELD: {
    }
    default: assert(0);
  }
}

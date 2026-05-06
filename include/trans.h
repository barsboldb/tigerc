#ifndef TRANS_H
#define TRANS_H

#include "symtab.h"
#include "ast.h"
#include "tree.h"
#include "frame.h"

typedef label_t *label_ref_t;

typedef struct patch_list_t {
  label_ref_t          head;
  struct patch_list_t *next;
} patch_list_t;

typedef struct {
  tree_stmt_t  *stm;
  patch_list_t *trues;
  patch_list_t *falses;
} cx_t;

typedef enum {
  TR_EX, 
  TR_NX,
  TR_CX,
} tr_exp_kind_t;

typedef struct tr_exp_t {
  tr_exp_kind_t kind;
  union {
    tree_expr_t *ex;
    tree_stmt_t *nx;
    cx_t         cx;
  };
} tr_exp_t;

tree_expr_t *un_ex(tr_exp_t *e);
tree_stmt_t *un_nx(tr_exp_t *e);
cx_t         un_cx(tr_exp_t *e);

void do_patch(patch_list_t *list, label_t label);

patch_list_t *patch_list_join(patch_list_t *a, patch_list_t *b);

tr_exp_t *tr_expr(symtab_t *aenv, frame_t *frame, expr_t *e);
tr_exp_t *tr_dec(symtab_t *aenv, frame_t *frame, dec_t *d);
tr_exp_t *tr_var(symtab_t *aenv, frame_t *frame, expr_t *e);

#endif

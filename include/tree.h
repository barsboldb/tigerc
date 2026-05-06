#ifndef TREE_H
#define TREE_H

#include "temp.h"

typedef enum {
  TREE_CONST,
  TREE_TEMP,
  TREE_BINOP,
  TREE_MEM,
  TREE_CALL,
  TREE_ESEQ,
  TREE_NAME,
} tree_expr_kind_t;


typedef enum {
  TREE_MOVE,
  TREE_EXP,
  TREE_JUMP,
  TREE_CJUMP,
  TREE_SEQ,
  TREE_LABEL,
} tree_stmt_kind_t;

typedef enum {
  TREE_EQ,
  TREE_NE,
  TREE_LT,
  TREE_GT,
  TREE_LE,
  TREE_GE,
} tree_relop_t;

typedef enum {
  TREE_ADD,
  TREE_SUB,
  TREE_MUL,
  TREE_DIV,
  TREE_AND,
  TREE_OR,
  TREE_XOR,
} tree_binop_t;

typedef struct tree_expr_t {
  tree_expr_kind_t kind;
  union {
    int const_;
    int temp;
    struct {
      tree_binop_t        op;
      struct tree_expr_t *e1;
      struct tree_expr_t *e2;
    } binop;
    struct tree_expr_t *mem;
    struct {
      struct tree_expr_t  *name;
      struct tree_expr_t **actuals;
      int                  num_actuals;
    } call;
    struct {
      struct tree_stmt_t *s;
      struct tree_expr_t *e;
    } eseq;
    label_t name;
  };
} tree_expr_t;

typedef struct tree_stmt_t {
  tree_stmt_kind_t kind;
  union {
    struct {
      struct tree_expr_t *d;
      struct tree_expr_t *s;
    } move;
    struct tree_expr_t *exp;
    struct {
      tree_expr_t  *target;
      label_t      *dests;
      int           num_dest;
    } jump_;
    struct {
      tree_relop_t op;
      tree_expr_t *e1;
      tree_expr_t *e2;
      label_t true_;
      label_t false_;
    } cjump;

    struct {
      struct tree_stmt_t *s1;
      struct tree_stmt_t *s2;
    } seq;

    label_t label;
  };
} tree_stmt_t;


tree_expr_t *tree_const(int val);
tree_expr_t *tree_temp(temp_t t);
tree_expr_t *tree_binop(tree_binop_t op, tree_expr_t *e1, tree_expr_t *e2);
tree_expr_t *tree_mem(tree_expr_t *e);
tree_expr_t *tree_call(tree_expr_t *name, tree_expr_t **actuals, int num_actuals);
tree_expr_t *tree_eseq(tree_stmt_t *s, tree_expr_t *e);
tree_expr_t *tree_name(label_t l);

tree_stmt_t *tree_move(tree_expr_t *d, tree_expr_t *s);
tree_stmt_t *tree_exp(tree_expr_t *e);
tree_stmt_t *tree_jump(tree_expr_t *target, label_t *dests, int num_dests);
tree_stmt_t *tree_cjump(tree_relop_t op, tree_expr_t *e1, tree_expr_t *e2, label_t true_, label_t false_);
tree_stmt_t *tree_seq(tree_stmt_t *s1, tree_stmt_t *s2);
tree_stmt_t *tree_label(label_t l);


#endif

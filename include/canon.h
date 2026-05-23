#ifndef CANON_H
#define CANON_H

#include "tree.h"

typedef struct stmt_list_t {
  tree_stmt_t        *stmt;
  struct stmt_list_t *next;
} stmt_list_t;

typedef struct block_t {
  stmt_list_t *stmts;
  struct block_t *next;
} block_t;

typedef struct {
  block_t *blocks;
  label_t  done;
} block_res_t;

stmt_list_t *linearize(tree_stmt_t *s);
block_res_t *basic_blocks(stmt_list_t *stmts);
stmt_list_t *trace_schedule(block_res_t *res);

#endif

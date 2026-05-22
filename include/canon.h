#ifndef CANON_H
#define CANON_H

#include "tree.h"

typedef struct stmt_list_t {
  tree_stmt_t        *stmt;
  struct stmt_list_t *next;
} stmt_list_t;

stmt_list_t *linearize(tree_stmt_t *s);

#endif

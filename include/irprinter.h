#ifndef IRPRINTER_H
#define IRPRINTER_H

#include "tree.h"

void print_tree_expr(tree_expr_t *e, int indent);
void print_tree_stmt(tree_stmt_t *s, int indent);
void print_frags(void);

#endif

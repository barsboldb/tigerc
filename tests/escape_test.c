#include "test.h"
#include "escape.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>

/* --- AST builder helpers --- */

static expr_t *make_int_expr(int val) {
  expr_t *e  = calloc(1, sizeof(expr_t));
  e->kind    = EXPR_INT;
  e->int_val = val;
  return e;
}

static expr_t *make_id_expr(char *id) {
  expr_t *e = calloc(1, sizeof(expr_t));
  e->kind   = EXPR_ID;
  e->id     = id;
  return e;
}

static expr_list_t *make_expr_list(expr_t *e, expr_list_t *next) {
  expr_list_t *l = malloc(sizeof(expr_list_t));
  l->expr = e;
  l->next = next;
  return l;
}

static dec_list_t *make_dec_list(dec_t *d, dec_list_t *next) {
  dec_list_t *l = malloc(sizeof(dec_list_t));
  l->dec  = d;
  l->next = next;
  return l;
}

static expr_t *make_let_expr(dec_list_t *decs, expr_list_t *body) {
  expr_t *e       = calloc(1, sizeof(expr_t));
  e->kind         = EXPR_LET;
  e->let.dec_list = decs;
  e->let.body     = body;
  return e;
}

static dec_t *make_var_dec(char *id, expr_t *init) {
  dec_t *d     = calloc(1, sizeof(dec_t));
  d->kind      = DEC_VAR;
  d->var.id    = id;
  d->var.init  = init;
  d->var.escape = 0;
  return d;
}

static param_t *make_param(char *name) {
  param_t *p  = calloc(1, sizeof(param_t));
  p->name     = name;
  p->escape   = 0;
  return p;
}

static param_list_t *make_param_list(param_t *p, param_list_t *next) {
  param_list_t *l = malloc(sizeof(param_list_t));
  l->param = p;
  l->next  = next;
  return l;
}

static dec_t *make_func_dec(char *id, param_list_t *args, expr_t *body) {
  dec_t *d       = calloc(1, sizeof(dec_t));
  d->kind        = DEC_FUNC;
  d->func.id     = id;
  d->func.args   = args;
  d->func.body   = body;
  return d;
}

static expr_t *make_for_expr(char *var, expr_t *init, expr_t *to, expr_t *body) {
  expr_t *e    = calloc(1, sizeof(expr_t));
  e->kind      = EXPR_FOR;
  e->for_.var  = var;
  e->for_.init = init;
  e->for_.to   = to;
  e->for_.body = body;
  e->for_.escape = 0;
  return e;
}

/* --- tests --- */

/* var x := 5; accessed at same depth → not escaped */
int test_escape_var_not_escaped() {
  dec_t *d = make_var_dec("x", make_int_expr(5));
  expr_t *prog = make_let_expr(
    make_dec_list(d, NULL),
    make_expr_list(make_id_expr("x"), NULL)
  );
  find_escape(prog);
  ASSERT_EQ(d->var.escape, 0);
  return 1;
}
REGISTER_TEST(test_escape_var_not_escaped);

/* var x := 5; accessed from nested function → escaped */
int test_escape_var_escapes_into_nested_func() {
  dec_t *d_var = make_var_dec("x", make_int_expr(5));
  dec_t *d_func = make_func_dec("f", NULL, make_id_expr("x"));
  expr_t *prog = make_let_expr(
    make_dec_list(d_var, make_dec_list(d_func, NULL)),
    make_expr_list(make_int_expr(0), NULL)
  );
  find_escape(prog);
  ASSERT_EQ(d_var->var.escape, 1);
  return 1;
}
REGISTER_TEST(test_escape_var_escapes_into_nested_func);

/* function param accessed only within its own body → not escaped */
int test_escape_param_not_escaped() {
  param_t *p = make_param("x");
  dec_t *d_func = make_func_dec("f",
    make_param_list(p, NULL),
    make_id_expr("x")
  );
  expr_t *prog = make_let_expr(
    make_dec_list(d_func, NULL),
    make_expr_list(make_int_expr(0), NULL)
  );
  find_escape(prog);
  ASSERT_EQ(p->escape, 0);
  return 1;
}
REGISTER_TEST(test_escape_param_not_escaped);

/* function param accessed from doubly-nested function → escaped */
int test_escape_param_escapes_into_nested_func() {
  param_t *p = make_param("x");
  /* inner function g() = x  — g is nested inside f */
  dec_t *d_g = make_func_dec("g", NULL, make_id_expr("x"));
  expr_t *f_body = make_let_expr(
    make_dec_list(d_g, NULL),
    make_expr_list(make_int_expr(0), NULL)
  );
  dec_t *d_f = make_func_dec("f", make_param_list(p, NULL), f_body);
  expr_t *prog = make_let_expr(
    make_dec_list(d_f, NULL),
    make_expr_list(make_int_expr(0), NULL)
  );
  find_escape(prog);
  ASSERT_EQ(p->escape, 1);
  return 1;
}
REGISTER_TEST(test_escape_param_escapes_into_nested_func);

/* for loop variable accessed only in body at same depth → not escaped */
int test_escape_for_var_not_escaped() {
  expr_t *for_expr = make_for_expr("i",
    make_int_expr(0), make_int_expr(10),
    make_id_expr("i")
  );
  expr_t *prog = make_let_expr(
    NULL,
    make_expr_list(for_expr, NULL)
  );
  find_escape(prog);
  ASSERT_EQ(for_expr->for_.escape, 0);
  return 1;
}
REGISTER_TEST(test_escape_for_var_not_escaped);

/* for loop variable accessed from nested function inside body → escaped */
int test_escape_for_var_escapes_into_nested_func() {
  dec_t *d_g = make_func_dec("g", NULL, make_id_expr("i"));
  expr_t *body = make_let_expr(
    make_dec_list(d_g, NULL),
    make_expr_list(make_int_expr(0), NULL)
  );
  expr_t *for_expr = make_for_expr("i",
    make_int_expr(0), make_int_expr(10),
    body
  );
  expr_t *prog = make_let_expr(
    NULL,
    make_expr_list(for_expr, NULL)
  );
  find_escape(prog);
  ASSERT_EQ(for_expr->for_.escape, 1);
  return 1;
}
REGISTER_TEST(test_escape_for_var_escapes_into_nested_func);

/* multiple vars: only the one captured escapes */
int test_escape_only_captured_var_escapes() {
  dec_t *d_x = make_var_dec("x", make_int_expr(1));
  dec_t *d_y = make_var_dec("y", make_int_expr(2));
  dec_t *d_f = make_func_dec("f", NULL, make_id_expr("x"));
  expr_t *prog = make_let_expr(
    make_dec_list(d_x, make_dec_list(d_y, make_dec_list(d_f, NULL))),
    make_expr_list(make_int_expr(0), NULL)
  );
  find_escape(prog);
  ASSERT_EQ(d_x->var.escape, 1);
  ASSERT_EQ(d_y->var.escape, 0);
  return 1;
}
REGISTER_TEST(test_escape_only_captured_var_escapes);

/* var captured by deeply nested (depth+2) function → escaped */
int test_escape_var_escapes_deeply_nested() {
  dec_t *d_var = make_var_dec("x", make_int_expr(0));
  dec_t *d_inner = make_func_dec("inner", NULL, make_id_expr("x"));
  expr_t *outer_body = make_let_expr(
    make_dec_list(d_inner, NULL),
    make_expr_list(make_int_expr(0), NULL)
  );
  dec_t *d_outer = make_func_dec("outer", NULL, outer_body);
  expr_t *prog = make_let_expr(
    make_dec_list(d_var, make_dec_list(d_outer, NULL)),
    make_expr_list(make_int_expr(0), NULL)
  );
  find_escape(prog);
  ASSERT_EQ(d_var->var.escape, 1);
  return 1;
}
REGISTER_TEST(test_escape_var_escapes_deeply_nested);

#include "test.h"
#include "trans.h"
#include "semant.h"
#include "frame.h"
#include "tree.h"
#include "ast.h"
#include "symtab.h"
#include <stdlib.h>
#include <string.h>

/* --- env helpers --- */

static symtab_t *make_aenv() {
  symtab_t *e = symtab_new(16);
  symtab_enter_scope(e);
  return e;
}

static frame_t *make_frame() {
  return frame_new("test", NULL, 0, 0);
}

static access_t *insert_local(symtab_t *aenv, frame_t *frame, char *name, int escapes) {
  access_t *a = frame_alloc_local(frame, escapes);
  symtab_insert(aenv, name, a);
  return a;
}

/* --- AST helpers --- */

static expr_t *make_int(int v) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_INT; e->int_val = v; e->line = e->col = 0;
  return e;
}

static expr_t *make_nil() {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_NIL; e->line = e->col = 0;
  return e;
}

static expr_t *make_string(char *s) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_STRING; e->str_val = s; e->line = e->col = 0;
  return e;
}

static expr_t *make_id(char *id) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_ID; e->id = id; e->line = e->col = 0;
  return e;
}

static expr_t *make_binop(binop_t op, expr_t *l, expr_t *r) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_BINOP; e->binop.op = op;
  e->binop.left = l; e->binop.right = r; e->line = e->col = 0;
  return e;
}

static expr_t *make_call(char *id, expr_list_t *args) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_CALL; e->call.id = id; e->call.arg_list = args;
  e->line = e->col = 0;
  return e;
}

static expr_list_t *make_elist(expr_t *expr, expr_list_t *next) {
  expr_list_t *l = malloc(sizeof(expr_list_t));
  l->expr = expr; l->next = next;
  return l;
}

static expr_t *make_seq(expr_list_t *seq) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_SEQ; e->seq = seq; e->line = e->col = 0;
  return e;
}

static expr_t *make_if(expr_t *cond, expr_t *then, expr_t *else_) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_IF; e->if_.cond = cond; e->if_.then = then; e->if_.else_ = else_;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_while(expr_t *cond, expr_t *body) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_WHILE; e->while_.cond = cond; e->while_.body = body;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_for(char *var, expr_t *init, expr_t *to, expr_t *body, int escape) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_FOR; e->for_.var = var; e->for_.init = init;
  e->for_.to = to; e->for_.body = body; e->for_.escape = escape;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_assign(expr_t *lhs, expr_t *rhs) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_ASSIGN; e->assign.lhs = lhs; e->assign.rhs = rhs;
  e->line = e->col = 0;
  return e;
}

static field_list_t *make_field(char *name, expr_t *val, field_list_t *next) {
  field_list_t *f = malloc(sizeof(field_list_t));
  f->name = name; f->val = val; f->next = next;
  return f;
}

static expr_t *make_record(char *type_name, field_list_t *fields) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_RECORD; e->record.type_name = type_name; e->record.fields = fields;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_array(char *type_name, expr_t *size, expr_t *init) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_ARRAY; e->array.type_name = type_name;
  e->array.size = size; e->array.init = init;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_index(expr_t *array, expr_t *index) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_INDEX; e->index_.array = array; e->index_.index = index;
  e->line = e->col = 0;
  return e;
}

/* ================================================================
   tr_expr tests
   ================================================================ */

int test_tr_expr_int() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_int(42));
  ASSERT(r != NULL);
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_CONST);
  ASSERT_EQ(r->ex->const_, 42);
  return 1;
}
REGISTER_TEST(test_tr_expr_int);

int test_tr_expr_nil() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_nil());
  ASSERT(r != NULL);
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_CONST);
  ASSERT_EQ(r->ex->const_, 0);
  return 1;
}
REGISTER_TEST(test_tr_expr_nil);

int test_tr_expr_string() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_string("hello"));
  ASSERT(r != NULL);
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_NAME);
  return 1;
}
REGISTER_TEST(test_tr_expr_string);

int test_tr_expr_binop_add() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_binop(OP_ADD, make_int(1), make_int(2)));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_BINOP);
  ASSERT_EQ(r->ex->binop.op, TREE_ADD);
  ASSERT_EQ(r->ex->binop.e1->const_, 1);
  ASSERT_EQ(r->ex->binop.e2->const_, 2);
  return 1;
}
REGISTER_TEST(test_tr_expr_binop_add);

int test_tr_expr_binop_sub() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_binop(OP_SUB, make_int(5), make_int(3)));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->binop.op, TREE_SUB);
  return 1;
}
REGISTER_TEST(test_tr_expr_binop_sub);

int test_tr_expr_binop_mul() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_binop(OP_MUL, make_int(3), make_int(4)));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->binop.op, TREE_MUL);
  return 1;
}
REGISTER_TEST(test_tr_expr_binop_mul);

/* Relational ops produce TR_CX with a CJUMP and patch lists */

int test_tr_expr_binop_eq() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_binop(OP_EQ, make_int(1), make_int(1)));
  ASSERT_EQ(r->kind, TR_CX);
  ASSERT(r->cx.stm != NULL);
  ASSERT_EQ(r->cx.stm->kind, TREE_CJUMP);
  ASSERT_EQ(r->cx.stm->cjump.op, TREE_EQ);
  ASSERT(r->cx.trues != NULL);
  ASSERT(r->cx.falses != NULL);
  return 1;
}
REGISTER_TEST(test_tr_expr_binop_eq);

int test_tr_expr_binop_lt() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_binop(OP_LT, make_int(1), make_int(2)));
  ASSERT_EQ(r->kind, TR_CX);
  ASSERT_EQ(r->cx.stm->cjump.op, TREE_LT);
  return 1;
}
REGISTER_TEST(test_tr_expr_binop_lt);

int test_tr_expr_seq_single() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_seq(make_elist(make_int(7), NULL)));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_CONST);
  ASSERT_EQ(r->ex->const_, 7);
  return 1;
}
REGISTER_TEST(test_tr_expr_seq_single);

int test_tr_expr_seq_multiple() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  expr_list_t *seq = make_elist(make_int(1), make_elist(make_int(2), NULL));
  tr_exp_t *r = tr_expr(aenv, f, make_seq(seq));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_ESEQ);
  /* last element (2) is the value */
  ASSERT_EQ(r->ex->eseq.e->kind, TREE_CONST);
  ASSERT_EQ(r->ex->eseq.e->const_, 2);
  return 1;
}
REGISTER_TEST(test_tr_expr_seq_multiple);

int test_tr_expr_call_no_args() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_call("foo", NULL));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_CALL);
  ASSERT_EQ(r->ex->call.num_actuals, 0);
  return 1;
}
REGISTER_TEST(test_tr_expr_call_no_args);

int test_tr_expr_call_with_args() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  expr_list_t *args = make_elist(make_int(1), make_elist(make_int(2), NULL));
  tr_exp_t *r = tr_expr(aenv, f, make_call("add", args));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_CALL);
  ASSERT_EQ(r->ex->call.num_actuals, 2);
  return 1;
}
REGISTER_TEST(test_tr_expr_call_with_args);

/* if-then (no else) is void → TR_NX */
int test_tr_expr_if_no_else() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_if(make_int(1), make_int(2), NULL));
  ASSERT_EQ(r->kind, TR_NX);
  ASSERT(r->nx != NULL);
  return 1;
}
REGISTER_TEST(test_tr_expr_if_no_else);

/* if-then-else has a value → TR_EX(ESEQ) */
int test_tr_expr_if_with_else() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_if(make_int(1), make_int(2), make_int(3)));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_ESEQ);
  return 1;
}
REGISTER_TEST(test_tr_expr_if_with_else);

int test_tr_expr_while() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_while(make_int(0), make_int(0)));
  ASSERT_EQ(r->kind, TR_NX);
  return 1;
}
REGISTER_TEST(test_tr_expr_while);

int test_tr_expr_for() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  expr_t *for_ = make_for("i", make_int(0), make_int(10), make_int(0), 0);
  tr_exp_t *r = tr_expr(aenv, f, for_);
  ASSERT_EQ(r->kind, TR_NX);
  return 1;
}
REGISTER_TEST(test_tr_expr_for);

static semty_t *make_record_ty(field_ty_t *fields) {
  semty_t *ty = malloc(sizeof(semty_t));
  ty->kind = SEMTY_RECORD;
  ty->record = fields;
  return ty;
}

static field_ty_t *make_field_ty(char *name, field_ty_t *next) {
  field_ty_t *f = malloc(sizeof(field_ty_t));
  f->name = name; f->type = NULL; f->next = next;
  return f;
}

/* EXPR_RECORD: TR_EX whose expression is ESEQ(alloc+inits, temp) */
int test_tr_expr_record_empty() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  expr_t *rec = make_record("point", NULL);
  rec->ty = make_record_ty(NULL);
  tr_exp_t *r = tr_expr(aenv, f, rec);
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_ESEQ);
  /* statement side: MOVE(temp, call(malloc, [0])) */
  tree_stmt_t *alloc = r->ex->eseq.s;
  ASSERT_EQ(alloc->kind, TREE_MOVE);
  ASSERT_EQ(alloc->move.s->kind, TREE_CALL);
  /* value side: the record pointer temp */
  ASSERT_EQ(r->ex->eseq.e->kind, TREE_TEMP);
  return 1;
}
REGISTER_TEST(test_tr_expr_record_empty);

int test_tr_expr_record_with_fields() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  field_list_t *fields = make_field("x", make_int(1), make_field("y", make_int(2), NULL));
  expr_t *rec = make_record("point", fields);
  rec->ty = make_record_ty(make_field_ty("x", make_field_ty("y", NULL)));
  tr_exp_t *r = tr_expr(aenv, f, rec);
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_ESEQ);
  /* statement: SEQ(malloc_move, field_inits) */
  ASSERT_EQ(r->ex->eseq.s->kind, TREE_SEQ);
  ASSERT_EQ(r->ex->eseq.e->kind, TREE_TEMP);
  return 1;
}
REGISTER_TEST(test_tr_expr_record_with_fields);

/* EXPR_ARRAY: TR_EX(CALL(init_array, [size, init])) */
int test_tr_expr_array() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  tr_exp_t *r = tr_expr(aenv, f, make_array("intarr", make_int(10), make_int(0)));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_CALL);
  ASSERT_EQ(r->ex->call.num_actuals, 2);
  /* first actual: size expr (CONST 10) */
  ASSERT_EQ(r->ex->call.actuals[0]->kind, TREE_CONST);
  ASSERT_EQ(r->ex->call.actuals[0]->const_, 10);
  /* second actual: init expr (CONST 0) */
  ASSERT_EQ(r->ex->call.actuals[1]->kind, TREE_CONST);
  ASSERT_EQ(r->ex->call.actuals[1]->const_, 0);
  return 1;
}
REGISTER_TEST(test_tr_expr_array);

/* type point = { x: int, y: int } — fields given as y=2, x=1 (reversed).
 * Correct translation must place x (val=1) at offset 0 and y (val=2) at
 * offset WORD_SIZE regardless of source order. Fails until tenv is threaded
 * through and field ordering is fixed. */
int test_tr_expr_record_canonical_field_order() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  field_list_t *fields = make_field("y", make_int(2), make_field("x", make_int(1), NULL));
  expr_t *rec = make_record("point", fields);
  rec->ty = make_record_ty(make_field_ty("x", make_field_ty("y", NULL)));
  tr_exp_t *r = tr_expr(aenv, f, rec);
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_ESEQ);
  /* eseq.s = SEQ(alloc, SEQ(x_init, y_init)) */
  tree_stmt_t *outer = r->ex->eseq.s;
  ASSERT_EQ(outer->kind, TREE_SEQ);
  tree_stmt_t *inits = outer->seq.s2; /* SEQ(x_init, y_init) */
  ASSERT_EQ(inits->kind, TREE_SEQ);
  /* first init: MOVE(MEM(r + 0), x_val=1) */
  tree_stmt_t *first = inits->seq.s1;
  ASSERT_EQ(first->kind, TREE_MOVE);
  ASSERT_EQ(first->move.d->kind, TREE_MEM);
  ASSERT_EQ(first->move.d->mem->binop.e2->const_, 0);    /* offset 0 → x */
  ASSERT_EQ(first->move.s->const_, 1);                   /* value 1 */
  /* second init: MOVE(MEM(r + WORD_SIZE), y_val=2) */
  tree_stmt_t *second = inits->seq.s2;
  ASSERT_EQ(second->kind, TREE_MOVE);
  ASSERT_EQ(second->move.d->kind, TREE_MEM);
  ASSERT_EQ(second->move.d->mem->binop.e2->const_, WORD_SIZE); /* offset WORD_SIZE → y */
  ASSERT_EQ(second->move.s->const_, 2);                        /* value 2 */
  return 1;
}
REGISTER_TEST(test_tr_expr_record_canonical_field_order);

int test_tr_expr_array_expr_size() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  /* size is a non-constant expression */
  expr_t *size = make_binop(OP_ADD, make_int(5), make_int(5));
  tr_exp_t *r = tr_expr(aenv, f, make_array("intarr", size, make_int(0)));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_CALL);
  ASSERT_EQ(r->ex->call.actuals[0]->kind, TREE_BINOP);
  return 1;
}
REGISTER_TEST(test_tr_expr_array_expr_size);

/* ================================================================
   tr_dec tests
   ================================================================ */

int test_tr_dec_type() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  dec_t *d = malloc(sizeof(dec_t));
  d->kind = DEC_TYPE; d->type.name = "myint";
  d->type.ty = malloc(sizeof(ty_t));
  d->type.ty->kind = TY_NAME; d->type.ty->alias = "int";
  tr_exp_t *r = tr_dec(aenv, f, d);
  /* type decs are no-ops in IR */
  ASSERT_EQ(r->kind, TR_NX);
  return 1;
}
REGISTER_TEST(test_tr_dec_type);

int test_tr_dec_var_allocates_and_moves() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  dec_t *d = malloc(sizeof(dec_t));
  d->kind = DEC_VAR; d->var.id = "x";
  d->var.type_name = NULL; d->var.init = make_int(5); d->var.escape = 0;
  tr_exp_t *r = tr_dec(aenv, f, d);
  ASSERT_EQ(r->kind, TR_NX);
  ASSERT_EQ(r->nx->kind, TREE_MOVE);
  /* source is the init value */
  ASSERT_EQ(r->nx->move.s->kind, TREE_CONST);
  ASSERT_EQ(r->nx->move.s->const_, 5);
  return 1;
}
REGISTER_TEST(test_tr_dec_var_allocates_and_moves);

int test_tr_dec_var_inserts_into_aenv() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  dec_t *d = malloc(sizeof(dec_t));
  d->kind = DEC_VAR; d->var.id = "x";
  d->var.type_name = NULL; d->var.init = make_int(5); d->var.escape = 0;
  tr_dec(aenv, f, d);
  access_t *a = symtab_lookup(aenv, "x");
  ASSERT(a != NULL);
  return 1;
}
REGISTER_TEST(test_tr_dec_var_inserts_into_aenv);

int test_tr_dec_func_is_noop() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  dec_t *d = malloc(sizeof(dec_t));
  d->kind = DEC_FUNC; d->func.id = "fn";
  d->func.type_name = NULL; d->func.args = NULL;
  d->func.body = make_int(0);
  tr_exp_t *r = tr_dec(aenv, f, d);
  ASSERT_EQ(r->kind, TR_NX);
  return 1;
}
REGISTER_TEST(test_tr_dec_func_is_noop);

/* ================================================================
   tr_var tests
   ================================================================ */

int test_tr_var_id_reg() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  insert_local(aenv, f, "x", 0); /* no escape → register */
  tr_exp_t *r = tr_var(aenv, f, make_id("x"));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_TEMP);
  return 1;
}
REGISTER_TEST(test_tr_var_id_reg);

int test_tr_var_id_frame() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  insert_local(aenv, f, "x", 1); /* escapes → frame slot */
  tr_exp_t *r = tr_var(aenv, f, make_id("x"));
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_MEM);
  return 1;
}
REGISTER_TEST(test_tr_var_id_frame);

int test_tr_var_index() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  insert_local(aenv, f, "a", 0);
  expr_t *e = make_index(make_id("a"), make_int(2));
  tr_exp_t *r = tr_var(aenv, f, e);
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_MEM);
  /* MEM(base + index * WORD_SIZE) */
  tree_expr_t *addr = r->ex->mem;
  ASSERT_EQ(addr->kind, TREE_BINOP);
  ASSERT_EQ(addr->binop.op, TREE_ADD);
  /* right side: index * WORD_SIZE */
  ASSERT_EQ(addr->binop.e2->kind, TREE_BINOP);
  ASSERT_EQ(addr->binop.e2->binop.op, TREE_MUL);
  ASSERT_EQ(addr->binop.e2->binop.e1->const_, 2);
  ASSERT_EQ(addr->binop.e2->binop.e2->const_, WORD_SIZE);
  return 1;
}
REGISTER_TEST(test_tr_var_index);

/* ================================================================
   un_ex / un_nx / un_cx conversion tests
   ================================================================ */

int test_un_ex_from_ex() {
  tree_expr_t *c = tree_const(99);
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_EX; e->ex = c;
  ASSERT_EQ(un_ex(e), c);
  return 1;
}
REGISTER_TEST(test_un_ex_from_ex);

int test_un_ex_from_nx() {
  tree_stmt_t *s = tree_exp(tree_const(0));
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_NX; e->nx = s;
  tree_expr_t *result = un_ex(e);
  /* NX → ESEQ(stm, CONST(0)) */
  ASSERT_EQ(result->kind, TREE_ESEQ);
  ASSERT_EQ(result->eseq.s, s);
  ASSERT_EQ(result->eseq.e->kind, TREE_CONST);
  ASSERT_EQ(result->eseq.e->const_, 0);
  return 1;
}
REGISTER_TEST(test_un_ex_from_nx);

int test_un_nx_from_ex() {
  tree_expr_t *c = tree_const(5);
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_EX; e->ex = c;
  tree_stmt_t *result = un_nx(e);
  ASSERT_EQ(result->kind, TREE_EXP);
  ASSERT_EQ(result->exp, c);
  return 1;
}
REGISTER_TEST(test_un_nx_from_ex);

int test_un_nx_from_nx() {
  tree_stmt_t *s = tree_exp(tree_const(0));
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_NX; e->nx = s;
  ASSERT_EQ(un_nx(e), s);
  return 1;
}
REGISTER_TEST(test_un_nx_from_nx);

int test_un_cx_from_ex() {
  tree_expr_t *c = tree_const(1);
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_EX; e->ex = c;
  cx_t cx = un_cx(e);
  ASSERT(cx.stm != NULL);
  ASSERT_EQ(cx.stm->kind, TREE_CJUMP);
  ASSERT_EQ(cx.stm->cjump.op, TREE_NE);
  ASSERT(cx.trues != NULL);
  ASSERT(cx.falses != NULL);
  return 1;
}
REGISTER_TEST(test_un_cx_from_ex);

int test_un_cx_from_cx() {
  tree_stmt_t *s = tree_cjump(TREE_LT, tree_const(1), tree_const(2), 0, 0);
  patch_list_t *trues  = malloc(sizeof(patch_list_t));
  trues->head = &s->cjump.true_; trues->next = NULL;
  patch_list_t *falses = malloc(sizeof(patch_list_t));
  falses->head = &s->cjump.false_; falses->next = NULL;
  tr_exp_t *e = malloc(sizeof(tr_exp_t));
  e->kind = TR_CX;
  e->cx = (cx_t){ s, trues, falses };
  cx_t result = un_cx(e);
  ASSERT_EQ(result.stm, s);
  ASSERT_EQ(result.trues, trues);
  ASSERT_EQ(result.falses, falses);
  return 1;
}
REGISTER_TEST(test_un_cx_from_cx);

int test_tr_var_field() {
  /* set up semant globals with a record type: point = { x: int, y: int } */
  tenv = semant_base_tenv();
  venv = semant_base_venv(tenv);
  dec_t *type_dec = malloc(sizeof(dec_t));
  type_dec->kind = DEC_TYPE;
  type_dec->type.name = "point";
  type_dec->type.ty   = malloc(sizeof(ty_t));
  type_dec->type.ty->kind   = TY_RECORD;
  param_t *px = malloc(sizeof(param_t)); px->name = "x"; px->type_name = "int";
  param_t *py = malloc(sizeof(param_t)); py->name = "y"; py->type_name = "int";
  param_list_t *py_l = malloc(sizeof(param_list_t)); py_l->param = py; py_l->next = NULL;
  param_list_t *px_l = malloc(sizeof(param_list_t)); px_l->param = px; px_l->next = py_l;
  type_dec->type.ty->fields = px_l;
  trans_dec(type_dec);

  /* register "r : point" in venv */
  semty_t *point_ty = symtab_lookup(tenv, "point");
  env_entry_t *ve = malloc(sizeof(env_entry_t));
  ve->kind = ENV_VAR; ve->var = point_ty;
  symtab_insert(venv, "r", ve);

  /* register "r" in aenv too */
  symtab_t *aenv = make_aenv();
  frame_t *fr = make_frame();
  insert_local(aenv, fr, "r", 0);

  expr_t *e = malloc(sizeof(expr_t));
  e->kind = EXPR_FIELD;
  e->field_.record = make_id("r");
  e->field_.record->ty = point_ty;
  e->field_.field  = "x";
  e->line = e->col = 0;

  tr_exp_t *r = tr_var(aenv, fr, e);
  ASSERT(r != NULL);
  ASSERT_EQ(r->kind, TR_EX);
  ASSERT_EQ(r->ex->kind, TREE_MEM);
  /* x is field 0 → offset 0 */
  ASSERT_EQ(r->ex->mem->binop.e2->const_, 0);
  return 1;
}
REGISTER_TEST(test_tr_var_field);

/* ================================================================
   assign test
   ================================================================ */

int test_tr_expr_assign() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  insert_local(aenv, f, "x", 0);
  tr_exp_t *r = tr_expr(aenv, f, make_assign(make_id("x"), make_int(42)));
  ASSERT_EQ(r->kind, TR_NX);
  ASSERT_EQ(r->nx->kind, TREE_MOVE);
  ASSERT_EQ(r->nx->move.s->kind, TREE_CONST);
  ASSERT_EQ(r->nx->move.s->const_, 42);
  return 1;
}
REGISTER_TEST(test_tr_expr_assign);

int test_tr_expr_field_assign() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  insert_local(aenv, f, "r", 1); /* escapes → frame slot */

  /* set up r->ty as a record with field "x" at index 0 */
  semty_t *field_ty = malloc(sizeof(semty_t)); field_ty->kind = SEMTY_INT;
  field_ty_t *ft = malloc(sizeof(field_ty_t));
  ft->name = "x"; ft->type = field_ty; ft->next = NULL;
  semty_t *rec_ty = malloc(sizeof(semty_t));
  rec_ty->kind = SEMTY_RECORD; rec_ty->record = ft;

  expr_t *rec_id = make_id("r");
  rec_id->ty = rec_ty;

  expr_t *field = malloc(sizeof(expr_t));
  field->kind = EXPR_FIELD; field->field_.record = rec_id;
  field->field_.field = "x"; field->line = field->col = 0;

  tr_exp_t *r = tr_expr(aenv, f, make_assign(field, make_int(99)));
  ASSERT_EQ(r->kind, TR_NX);
  ASSERT_EQ(r->nx->kind, TREE_MOVE);
  ASSERT_EQ(r->nx->move.d->kind, TREE_MEM); /* lhs is a memory location */
  ASSERT_EQ(r->nx->move.s->kind, TREE_CONST);
  ASSERT_EQ(r->nx->move.s->const_, 99);
  return 1;
}
REGISTER_TEST(test_tr_expr_field_assign);

int test_tr_expr_index_assign() {
  symtab_t *aenv = make_aenv();
  frame_t *f = make_frame();
  insert_local(aenv, f, "a", 0);
  expr_t *lhs = make_index(make_id("a"), make_int(2));
  tr_exp_t *r = tr_expr(aenv, f, make_assign(lhs, make_int(7)));
  ASSERT_EQ(r->kind, TR_NX);
  ASSERT_EQ(r->nx->kind, TREE_MOVE);
  ASSERT_EQ(r->nx->move.d->kind, TREE_MEM); /* lhs is a memory location */
  ASSERT_EQ(r->nx->move.s->kind, TREE_CONST);
  ASSERT_EQ(r->nx->move.s->const_, 7);
  return 1;
}
REGISTER_TEST(test_tr_expr_index_assign);

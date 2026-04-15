#include "test.h"
#include "semant.h"
#include "symtab.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>

static symtab_t *base_tenv() {
  symtab_t *tenv = symtab_new(16);
  semty_t *int_ty    = malloc(sizeof(semty_t));
  semty_t *string_ty = malloc(sizeof(semty_t));
  int_ty->kind    = SEMTY_INT;
  string_ty->kind = SEMTY_STRING;
  symtab_enter_scope(tenv);
  symtab_insert(tenv, "int",    int_ty);
  symtab_insert(tenv, "string", string_ty);
  return tenv;
}

static ty_t *make_name_ty(char *name) {
  ty_t *ty  = malloc(sizeof(ty_t));
  ty->kind  = TY_NAME;
  ty->alias = name;
  return ty;
}

static ty_t *make_array_ty(char *elem) {
  ty_t *ty      = malloc(sizeof(ty_t));
  ty->kind      = TY_ARRAY;
  ty->array_of  = elem;
  return ty;
}

static ty_t *make_record_ty(param_list_t *fields) {
  ty_t *ty  = malloc(sizeof(ty_t));
  ty->kind  = TY_RECORD;
  ty->fields = fields;
  return ty;
}

static param_list_t *make_param(char *name, char *type_name, param_list_t *next) {
  param_t *p      = malloc(sizeof(param_t));
  p->name         = name;
  p->type_name    = type_name;
  param_list_t *l = malloc(sizeof(param_list_t));
  l->param        = p;
  l->next         = next;
  return l;
}

int test_trans_ty_name_int() {
  symtab_t *tenv = base_tenv();
  ty_t *ty = make_name_ty("int");
  semty_t *result = trans_ty(tenv, ty);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_ty_name_int);

int test_trans_ty_name_string() {
  symtab_t *tenv = base_tenv();
  ty_t *ty = make_name_ty("string");
  semty_t *result = trans_ty(tenv, ty);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_STRING);
  return 1;
}
REGISTER_TEST(test_trans_ty_name_string);

int test_trans_ty_name_unknown() {
  symtab_t *tenv = base_tenv();
  ty_t *ty = make_name_ty("unknown");
  semty_t *result = trans_ty(tenv, ty);
  ASSERT_EQ(result, NULL);
  return 1;
}
REGISTER_TEST(test_trans_ty_name_unknown);

int test_trans_ty_array_of_int() {
  symtab_t *tenv = base_tenv();
  ty_t *ty = make_array_ty("int");
  semty_t *result = trans_ty(tenv, ty);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_ARRAY);
  ASSERT(result->array != NULL);
  ASSERT_EQ(result->array->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_ty_array_of_int);

int test_trans_ty_array_of_string() {
  symtab_t *tenv = base_tenv();
  ty_t *ty = make_array_ty("string");
  semty_t *result = trans_ty(tenv, ty);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_ARRAY);
  ASSERT_EQ(result->array->kind, SEMTY_STRING);
  return 1;
}
REGISTER_TEST(test_trans_ty_array_of_string);

int test_trans_ty_record_empty() {
  symtab_t *tenv = base_tenv();
  ty_t *ty = make_record_ty(NULL);
  semty_t *result = trans_ty(tenv, ty);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_RECORD);
  ASSERT_EQ(result->record, NULL);
  return 1;
}
REGISTER_TEST(test_trans_ty_record_empty);

int test_trans_ty_record_single_field() {
  symtab_t *tenv = base_tenv();
  param_list_t *fields = make_param("x", "int", NULL);
  ty_t *ty = make_record_ty(fields);
  semty_t *result = trans_ty(tenv, ty);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_RECORD);
  ASSERT(result->record != NULL);
  ASSERT_STR_EQ(result->record->name, "x");
  ASSERT_EQ(result->record->type->kind, SEMTY_INT);
  ASSERT_EQ(result->record->next, NULL);
  return 1;
}
REGISTER_TEST(test_trans_ty_record_single_field);

int test_trans_ty_record_multiple_fields() {
  symtab_t *tenv = base_tenv();
  param_list_t *fields = make_param("x", "int", make_param("s", "string", NULL));
  ty_t *ty = make_record_ty(fields);
  semty_t *result = trans_ty(tenv, ty);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_RECORD);
  field_ty_t *f = result->record;
  ASSERT_STR_EQ(f->name, "x");
  ASSERT_EQ(f->type->kind, SEMTY_INT);
  f = f->next;
  ASSERT(f != NULL);
  ASSERT_STR_EQ(f->name, "s");
  ASSERT_EQ(f->type->kind, SEMTY_STRING);
  ASSERT_EQ(f->next, NULL);
  return 1;
}
REGISTER_TEST(test_trans_ty_record_multiple_fields);

/* --- helpers for trans_dec tests --- */

static symtab_t *base_venv() {
  symtab_t *venv = symtab_new(16);
  symtab_enter_scope(venv);
  return venv;
}

static dec_t *make_type_dec(char *name, ty_t *ty) {
  dec_t *d    = malloc(sizeof(dec_t));
  d->kind     = DEC_TYPE;
  d->type.name = name;
  d->type.ty   = ty;
  return d;
}

static dec_t *make_func_dec(char *id, param_list_t *args, char *ret_type) {
  dec_t *d       = malloc(sizeof(dec_t));
  d->kind        = DEC_FUNC;
  d->func.id     = id;
  d->func.args   = args;
  d->func.type_name = ret_type;
  d->func.body   = NULL;
  return d;
}

/* --- trans_dec: DEC_TYPE --- */

int test_trans_dec_type_alias() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = make_type_dec("myint", make_name_ty("int"));
  trans_dec(venv, tenv, d);
  semty_t *result = symtab_lookup(tenv, "myint");
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_dec_type_alias);

int test_trans_dec_type_array() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = make_type_dec("intarr", make_array_ty("int"));
  trans_dec(venv, tenv, d);
  semty_t *result = symtab_lookup(tenv, "intarr");
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_ARRAY);
  ASSERT_EQ(result->array->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_dec_type_array);

int test_trans_dec_type_record() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  param_list_t *fields = make_param("x", "int", make_param("s", "string", NULL));
  dec_t *d = make_type_dec("point", make_record_ty(fields));
  trans_dec(venv, tenv, d);
  semty_t *result = symtab_lookup(tenv, "point");
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_RECORD);
  ASSERT_STR_EQ(result->record->name, "x");
  ASSERT_EQ(result->record->type->kind, SEMTY_INT);
  ASSERT_STR_EQ(result->record->next->name, "s");
  ASSERT_EQ(result->record->next->type->kind, SEMTY_STRING);
  return 1;
}
REGISTER_TEST(test_trans_dec_type_record);

/* --- trans_dec: DEC_FUNC --- */

int test_trans_dec_func_no_params_no_ret() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = make_func_dec("f", NULL, NULL);
  trans_dec_header(venv, tenv, d);
  trans_dec(venv, tenv, d);
  env_entry_t *entry = symtab_lookup(venv, "f");
  ASSERT(entry != NULL);
  ASSERT_EQ(entry->kind, ENV_FUNC);
  ASSERT_EQ(entry->func.params, NULL);
  ASSERT_EQ(entry->func.ret, NULL);
  return 1;
}
REGISTER_TEST(test_trans_dec_func_no_params_no_ret);

int test_trans_dec_func_with_return_type() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = make_func_dec("f", NULL, "int");
  trans_dec_header(venv, tenv, d);
  trans_dec(venv, tenv, d);
  env_entry_t *entry = symtab_lookup(venv, "f");
  ASSERT(entry != NULL);
  ASSERT_EQ(entry->func.ret->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_dec_func_with_return_type);

int test_trans_dec_func_with_params() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  param_list_t *args = make_param("x", "int", make_param("s", "string", NULL));
  dec_t *d = make_func_dec("f", args, "int");
  trans_dec_header(venv, tenv, d);
  trans_dec(venv, tenv, d);
  env_entry_t *entry = symtab_lookup(venv, "f");
  ASSERT(entry != NULL);
  param_ty_t *p = entry->func.params;
  ASSERT(p != NULL);
  ASSERT_EQ(p->type->kind, SEMTY_INT);
  p = p->next;
  ASSERT(p != NULL);
  ASSERT_EQ(p->type->kind, SEMTY_STRING);
  ASSERT_EQ(p->next, NULL);
  return 1;
}
REGISTER_TEST(test_trans_dec_func_with_params);

int test_trans_dec_func_inserted_in_venv() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = make_func_dec("myfunc", NULL, NULL);
  trans_dec_header(venv, tenv, d);
  trans_dec(venv, tenv, d);
  ASSERT(symtab_lookup(venv, "myfunc") != NULL);
  ASSERT_EQ(symtab_lookup(venv, "unknown"), NULL);
  return 1;
}
REGISTER_TEST(test_trans_dec_func_inserted_in_venv);

/* --- helpers for trans_expr tests --- */

static expr_t *make_int_expr(int val) {
  expr_t *e   = malloc(sizeof(expr_t));
  e->kind     = EXPR_INT;
  e->int_val  = val;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_string_expr(char *val) {
  expr_t *e   = malloc(sizeof(expr_t));
  e->kind     = EXPR_STRING;
  e->str_val  = val;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_nil_expr() {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind   = EXPR_NIL;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_binop_expr(binop_t op, expr_t *left, expr_t *right) {
  expr_t *e       = malloc(sizeof(expr_t));
  e->kind         = EXPR_BINOP;
  e->binop.op     = op;
  e->binop.left   = left;
  e->binop.right  = right;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_if_expr(expr_t *cond, expr_t *then, expr_t *else_) {
  expr_t *e     = malloc(sizeof(expr_t));
  e->kind       = EXPR_IF;
  e->if_.cond   = cond;
  e->if_.then   = then;
  e->if_.else_  = else_;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_while_expr(expr_t *cond, expr_t *body) {
  expr_t *e      = malloc(sizeof(expr_t));
  e->kind        = EXPR_WHILE;
  e->while_.cond = cond;
  e->while_.body = body;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_for_expr(expr_t *init, expr_t *to, expr_t *body) {
  expr_t *e    = malloc(sizeof(expr_t));
  e->kind      = EXPR_FOR;
  e->for_.var  = "i";
  e->for_.init = init;
  e->for_.to   = to;
  e->for_.body = body;
  e->line = e->col = 0;
  return e;
}

static expr_list_t *make_expr_list(expr_t *e, expr_list_t *next) {
  expr_list_t *l = malloc(sizeof(expr_list_t));
  l->expr = e;
  l->next = next;
  return l;
}

static expr_t *make_seq_expr(expr_list_t *seq) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind   = EXPR_SEQ;
  e->seq    = seq;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_call_expr(char *id, expr_list_t *args) {
  expr_t *e        = malloc(sizeof(expr_t));
  e->kind          = EXPR_CALL;
  e->call.id       = id;
  e->call.arg_list = args;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_array_expr(char *type_name, expr_t *size, expr_t *init) {
  expr_t *e          = malloc(sizeof(expr_t));
  e->kind            = EXPR_ARRAY;
  e->array.type_name = type_name;
  e->array.size      = size;
  e->array.init      = init;
  e->line = e->col   = 0;
  return e;
}

static field_list_t *make_field_list(char *name, expr_t *val, field_list_t *next) {
  field_list_t *f = malloc(sizeof(field_list_t));
  f->name = name;
  f->val  = val;
  f->next = next;
  return f;
}

static expr_t *make_record_expr(char *type_name, field_list_t *fields) {
  expr_t *e            = malloc(sizeof(expr_t));
  e->kind              = EXPR_RECORD;
  e->record.type_name  = type_name;
  e->record.fields     = fields;
  e->line = e->col     = 0;
  return e;
}

/* --- trans_expr tests --- */

int test_trans_expr_int() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  semty_t *result = trans_expr(venv, tenv, make_int_expr(42));
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_expr_int);

int test_trans_expr_string() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  semty_t *result = trans_expr(venv, tenv, make_string_expr("hello"));
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_STRING);
  return 1;
}
REGISTER_TEST(test_trans_expr_string);

int test_trans_expr_nil() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  semty_t *result = trans_expr(venv, tenv, make_nil_expr());
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_NIL);
  return 1;
}
REGISTER_TEST(test_trans_expr_nil);

int test_trans_expr_binop_add() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  expr_t *e = make_binop_expr(OP_ADD, make_int_expr(1), make_int_expr(2));
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_expr_binop_add);

int test_trans_expr_binop_eq_ints() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  expr_t *e = make_binop_expr(OP_EQ, make_int_expr(1), make_int_expr(1));
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_expr_binop_eq_ints);

int test_trans_expr_binop_eq_strings() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  expr_t *e = make_binop_expr(OP_EQ, make_string_expr("a"), make_string_expr("b"));
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_expr_binop_eq_strings);

int test_trans_expr_if_no_else() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  expr_t *e = make_if_expr(make_int_expr(1), make_int_expr(2), NULL);
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_expr_if_no_else);

int test_trans_expr_if_with_else() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  expr_t *e = make_if_expr(make_int_expr(1), make_int_expr(2), make_int_expr(3));
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_expr_if_with_else);

int test_trans_expr_while() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  expr_t *e = make_while_expr(make_int_expr(1), make_int_expr(0));
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT_EQ(result->kind, SEMTY_VOID);
  return 1;
}
REGISTER_TEST(test_trans_expr_while);

int test_trans_expr_for() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  expr_t *e = make_for_expr(make_int_expr(0), make_int_expr(10), make_int_expr(0));
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT_EQ(result->kind, SEMTY_VOID);
  return 1;
}
REGISTER_TEST(test_trans_expr_for);

int test_trans_expr_seq() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  expr_list_t *seq = make_expr_list(make_int_expr(1),
                     make_expr_list(make_string_expr("x"), NULL));
  expr_t *e = make_seq_expr(seq);
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT_EQ(result->kind, SEMTY_STRING);
  return 1;
}
REGISTER_TEST(test_trans_expr_seq);

int test_trans_expr_call_no_args() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = make_func_dec("f", NULL, "int");
  trans_dec_header(venv, tenv, d);
  trans_dec(venv, tenv, d);
  expr_t *e = make_call_expr("f", NULL);
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_expr_call_no_args);

int test_trans_expr_call_with_args() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  param_list_t *params = make_param("x", "int", NULL);
  dec_t *d = make_func_dec("f", params, "string");
  trans_dec_header(venv, tenv, d);
  trans_dec(venv, tenv, d);
  expr_list_t *args = make_expr_list(make_int_expr(1), NULL);
  expr_t *e = make_call_expr("f", args);
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_STRING);
  return 1;
}
REGISTER_TEST(test_trans_expr_call_with_args);

int test_trans_expr_array() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  trans_dec(venv, tenv, make_type_dec("intarr", make_array_ty("int")));
  expr_t *e = make_array_expr("intarr", make_int_expr(10), make_int_expr(0));
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_ARRAY);
  ASSERT_EQ(result->array->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_expr_array);

int test_trans_expr_record() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  param_list_t *fields = make_param("x", "int", make_param("s", "string", NULL));
  trans_dec(venv, tenv, make_type_dec("point", make_record_ty(fields)));
  field_list_t *flist = make_field_list("s", make_string_expr("hi"),
                        make_field_list("x", make_int_expr(1), NULL));
  expr_t *e = make_record_expr("point", flist);
  semty_t *result = trans_expr(venv, tenv, e);
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_RECORD);
  return 1;
}
REGISTER_TEST(test_trans_expr_record);

/* --- helpers for trans_var tests --- */

static expr_t *make_id_expr(char *id) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind   = EXPR_ID;
  e->id     = id;
  e->line = e->col = 0;
  return e;
}

static expr_t *make_field_expr(expr_t *record, char *field) {
  expr_t *e         = malloc(sizeof(expr_t));
  e->kind           = EXPR_FIELD;
  e->field_.record  = record;
  e->field_.field   = field;
  e->line = e->col  = 0;
  return e;
}

static expr_t *make_index_expr(expr_t *array, expr_t *index) {
  expr_t *e         = malloc(sizeof(expr_t));
  e->kind           = EXPR_INDEX;
  e->index_.array   = array;
  e->index_.index   = index;
  e->line = e->col  = 0;
  return e;
}

static void insert_var(symtab_t *venv, char *name, semty_t *ty) {
  env_entry_t *e = malloc(sizeof(env_entry_t));
  e->kind = ENV_VAR;
  e->var  = ty;
  symtab_insert(venv, name, e);
}

/* --- trans_var tests --- */

int test_trans_var_id() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  semty_t *int_ty = symtab_lookup(tenv, "int");
  insert_var(venv, "x", int_ty);
  semty_t *result = trans_var(venv, tenv, make_id_expr("x"));
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_var_id);

int test_trans_var_id_undefined() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  semty_t *result = trans_var(venv, tenv, make_id_expr("x"));
  ASSERT_EQ(result, NULL);
  return 1;
}
REGISTER_TEST(test_trans_var_id_undefined);

int test_trans_var_id_function() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *fd = make_func_dec("f", NULL, "int");
  trans_dec_header(venv, tenv, fd);
  trans_dec(venv, tenv, fd);
  semty_t *result = trans_var(venv, tenv, make_id_expr("f"));
  ASSERT_EQ(result, NULL);
  return 1;
}
REGISTER_TEST(test_trans_var_id_function);

int test_trans_var_field() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  param_list_t *fields = make_param("x", "int", make_param("s", "string", NULL));
  trans_dec(venv, tenv, make_type_dec("point", make_record_ty(fields)));
  semty_t *point_ty = symtab_lookup(tenv, "point");
  insert_var(venv, "p", point_ty);
  semty_t *result = trans_var(venv, tenv, make_field_expr(make_id_expr("p"), "x"));
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_var_field);

int test_trans_var_field_unknown() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  param_list_t *fields = make_param("x", "int", NULL);
  trans_dec(venv, tenv, make_type_dec("point", make_record_ty(fields)));
  semty_t *point_ty = symtab_lookup(tenv, "point");
  insert_var(venv, "p", point_ty);
  semty_t *result = trans_var(venv, tenv, make_field_expr(make_id_expr("p"), "y"));
  ASSERT_EQ(result, NULL);
  return 1;
}
REGISTER_TEST(test_trans_var_field_unknown);

int test_trans_var_index() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  trans_dec(venv, tenv, make_type_dec("intarr", make_array_ty("int")));
  semty_t *arr_ty = symtab_lookup(tenv, "intarr");
  insert_var(venv, "a", arr_ty);
  semty_t *result = trans_var(venv, tenv, make_index_expr(make_id_expr("a"), make_int_expr(0)));
  ASSERT(result != NULL);
  ASSERT_EQ(result->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_var_index);

int test_trans_var_index_non_int() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  trans_dec(venv, tenv, make_type_dec("intarr", make_array_ty("int")));
  semty_t *arr_ty = symtab_lookup(tenv, "intarr");
  insert_var(venv, "a", arr_ty);
  semty_t *result = trans_var(venv, tenv, make_index_expr(make_id_expr("a"), make_string_expr("bad")));
  ASSERT_EQ(result, NULL);
  return 1;
}
REGISTER_TEST(test_trans_var_index_non_int);

int test_trans_var_field_on_non_record() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  semty_t *int_ty = symtab_lookup(tenv, "int");
  insert_var(venv, "x", int_ty);
  semty_t *result = trans_var(venv, tenv, make_field_expr(make_id_expr("x"), "foo"));
  ASSERT_EQ(result, NULL);
  return 1;
}
REGISTER_TEST(test_trans_var_field_on_non_record);

int test_trans_var_index_on_non_array() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  semty_t *int_ty = symtab_lookup(tenv, "int");
  insert_var(venv, "x", int_ty);
  semty_t *result = trans_var(venv, tenv, make_index_expr(make_id_expr("x"), make_int_expr(0)));
  ASSERT_EQ(result, NULL);
  return 1;
}
REGISTER_TEST(test_trans_var_index_on_non_array);

/* --- trans_dec: DEC_VAR --- */

int test_trans_dec_var_no_annotation() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = malloc(sizeof(dec_t));
  d->kind          = DEC_VAR;
  d->var.id        = "x";
  d->var.type_name = NULL;
  d->var.init      = make_int_expr(5);
  trans_dec(venv, tenv, d);
  env_entry_t *entry = symtab_lookup(venv, "x");
  ASSERT(entry != NULL);
  ASSERT_EQ(entry->kind, ENV_VAR);
  ASSERT_EQ(entry->var->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_dec_var_no_annotation);

int test_trans_dec_var_with_annotation() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = malloc(sizeof(dec_t));
  d->kind          = DEC_VAR;
  d->var.id        = "x";
  d->var.type_name = "int";
  d->var.init      = make_int_expr(5);
  trans_dec(venv, tenv, d);
  env_entry_t *entry = symtab_lookup(venv, "x");
  ASSERT(entry != NULL);
  ASSERT_EQ(entry->var->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_dec_var_with_annotation);

int test_trans_dec_var_string() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();
  dec_t *d = malloc(sizeof(dec_t));
  d->kind          = DEC_VAR;
  d->var.id        = "s";
  d->var.type_name = NULL;
  d->var.init      = make_string_expr("hello");
  trans_dec(venv, tenv, d);
  env_entry_t *entry = symtab_lookup(venv, "s");
  ASSERT(entry != NULL);
  ASSERT_EQ(entry->var->kind, SEMTY_STRING);
  return 1;
}
REGISTER_TEST(test_trans_dec_var_string);

/* --- trans_dec: recursive and mutually recursive functions --- */

// function fib(n: int): int = if n <= 1 then n else fib(n-1) + fib(n-2)
int test_trans_dec_func_recursive() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();

  expr_t *call1 = make_call_expr("fib",
    make_expr_list(make_binop_expr(OP_SUB, make_id_expr("n"), make_int_expr(1)), NULL));
  expr_t *call2 = make_call_expr("fib",
    make_expr_list(make_binop_expr(OP_SUB, make_id_expr("n"), make_int_expr(2)), NULL));
  expr_t *body = make_if_expr(
    make_binop_expr(OP_LE, make_id_expr("n"), make_int_expr(1)),
    make_id_expr("n"),
    make_binop_expr(OP_ADD, call1, call2));

  dec_t *d = malloc(sizeof(dec_t));
  d->kind           = DEC_FUNC;
  d->func.id        = "fib";
  d->func.type_name = "int";
  d->func.args      = make_param("n", "int", NULL);
  d->func.body      = body;

  trans_dec_header(venv, tenv, d);
  trans_dec(venv, tenv, d);
  env_entry_t *entry = symtab_lookup(venv, "fib");
  ASSERT(entry != NULL);
  ASSERT_EQ(entry->kind, ENV_FUNC);
  ASSERT(entry->func.ret != NULL);
  ASSERT_EQ(entry->func.ret->kind, SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_dec_func_recursive);

// function isEven(n: int): int = if n = 0 then 1 else isOdd(n-1)
// function isOdd(n: int): int  = if n = 0 then 0 else isEven(n-1)
int test_trans_dec_func_mutually_recursive() {
  symtab_t *tenv = base_tenv();
  symtab_t *venv = base_venv();

  expr_t *even_body = make_if_expr(
    make_binop_expr(OP_EQ, make_id_expr("n"), make_int_expr(0)),
    make_int_expr(1),
    make_call_expr("isOdd",
      make_expr_list(make_binop_expr(OP_SUB, make_id_expr("n"), make_int_expr(1)), NULL)));

  expr_t *odd_body = make_if_expr(
    make_binop_expr(OP_EQ, make_id_expr("n"), make_int_expr(0)),
    make_int_expr(0),
    make_call_expr("isEven",
      make_expr_list(make_binop_expr(OP_SUB, make_id_expr("n"), make_int_expr(1)), NULL)));

  dec_t *d_even = malloc(sizeof(dec_t));
  d_even->kind           = DEC_FUNC;
  d_even->func.id        = "isEven";
  d_even->func.type_name = "int";
  d_even->func.args      = make_param("n", "int", NULL);
  d_even->func.body      = even_body;

  dec_t *d_odd = malloc(sizeof(dec_t));
  d_odd->kind           = DEC_FUNC;
  d_odd->func.id        = "isOdd";
  d_odd->func.type_name = "int";
  d_odd->func.args      = make_param("n", "int", NULL);
  d_odd->func.body      = odd_body;

  trans_dec_header(venv, tenv, d_even);
  trans_dec_header(venv, tenv, d_odd);
  trans_dec(venv, tenv, d_even);
  trans_dec(venv, tenv, d_odd);

  env_entry_t *even_entry = symtab_lookup(venv, "isEven");
  env_entry_t *odd_entry  = symtab_lookup(venv, "isOdd");
  ASSERT(even_entry != NULL);
  ASSERT(odd_entry  != NULL);
  ASSERT_EQ(even_entry->func.ret->kind, SEMTY_INT);
  ASSERT_EQ(odd_entry->func.ret->kind,  SEMTY_INT);
  return 1;
}
REGISTER_TEST(test_trans_dec_func_mutually_recursive);

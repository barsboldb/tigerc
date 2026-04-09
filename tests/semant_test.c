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

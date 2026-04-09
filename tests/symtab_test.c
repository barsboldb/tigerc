#include "test.h"
#include "symtab.h"
#include <string.h>

static int x = 1, y = 2, z = 3;

int test_symtab_basic_insert_lookup() {
  symtab_t *tab = symtab_new(16);
  symtab_insert(tab, "x", &x);
  ASSERT_EQ(symtab_lookup(tab, "x"), &x);
  return 1;
}
REGISTER_TEST(test_symtab_basic_insert_lookup);

int test_symtab_lookup_missing() {
  symtab_t *tab = symtab_new(16);
  ASSERT_EQ(symtab_lookup(tab, "x"), NULL);
  return 1;
}
REGISTER_TEST(test_symtab_lookup_missing);

int test_symtab_overwrite() {
  symtab_t *tab = symtab_new(16);
  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &x);
  symtab_insert(tab, "x", &y);
  ASSERT_EQ(symtab_lookup(tab, "x"), &y);
  return 1;
}
REGISTER_TEST(test_symtab_overwrite);

int test_symtab_scope_shadow_and_restore() {
  symtab_t *tab = symtab_new(16);
  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &x);

  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &y);
  ASSERT_EQ(symtab_lookup(tab, "x"), &y);
  symtab_exit_scope(tab);

  ASSERT_EQ(symtab_lookup(tab, "x"), &x);
  return 1;
}
REGISTER_TEST(test_symtab_scope_shadow_and_restore);

int test_symtab_exit_scope_removes_new_binding() {
  symtab_t *tab = symtab_new(16);
  symtab_enter_scope(tab);
  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &x);
  symtab_exit_scope(tab);
  ASSERT_EQ(symtab_lookup(tab, "x"), NULL);
  return 1;
}
REGISTER_TEST(test_symtab_exit_scope_removes_new_binding);

int test_symtab_multiple_keys() {
  symtab_t *tab = symtab_new(16);
  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &x);
  symtab_insert(tab, "y", &y);
  symtab_insert(tab, "z", &z);
  ASSERT_EQ(symtab_lookup(tab, "x"), &x);
  ASSERT_EQ(symtab_lookup(tab, "y"), &y);
  ASSERT_EQ(symtab_lookup(tab, "z"), &z);
  return 1;
}
REGISTER_TEST(test_symtab_multiple_keys);

int test_symtab_nested_scopes() {
  symtab_t *tab = symtab_new(16);
  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &x);

  symtab_enter_scope(tab);
  symtab_insert(tab, "y", &y);

  symtab_enter_scope(tab);
  symtab_insert(tab, "z", &z);
  ASSERT_EQ(symtab_lookup(tab, "x"), &x);
  ASSERT_EQ(symtab_lookup(tab, "y"), &y);
  ASSERT_EQ(symtab_lookup(tab, "z"), &z);
  symtab_exit_scope(tab);

  ASSERT_EQ(symtab_lookup(tab, "z"), NULL);
  ASSERT_EQ(symtab_lookup(tab, "y"), &y);
  symtab_exit_scope(tab);

  ASSERT_EQ(symtab_lookup(tab, "y"), NULL);
  ASSERT_EQ(symtab_lookup(tab, "x"), &x);
  return 1;
}
REGISTER_TEST(test_symtab_nested_scopes);

/* Force a hash collision by using a small table (size=1) */
int test_symtab_hash_collision() {
  symtab_t *tab = symtab_new(1);
  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &x);
  symtab_insert(tab, "y", &y);
  symtab_insert(tab, "z", &z);
  ASSERT_EQ(symtab_lookup(tab, "x"), &x);
  ASSERT_EQ(symtab_lookup(tab, "y"), &y);
  ASSERT_EQ(symtab_lookup(tab, "z"), &z);
  return 1;
}
REGISTER_TEST(test_symtab_hash_collision);

int test_symtab_collision_scope_restore() {
  symtab_t *tab = symtab_new(1);
  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &x);
  symtab_insert(tab, "y", &y);

  symtab_enter_scope(tab);
  symtab_insert(tab, "x", &z);
  ASSERT_EQ(symtab_lookup(tab, "x"), &z);
  symtab_exit_scope(tab);

  ASSERT_EQ(symtab_lookup(tab, "x"), &x);
  ASSERT_EQ(symtab_lookup(tab, "y"), &y);
  return 1;
}
REGISTER_TEST(test_symtab_collision_scope_restore);

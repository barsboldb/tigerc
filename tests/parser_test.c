#include "test.h"
#include "parser.h"
#include "ast.h"
#include <string.h>

static expr_t *parse_str(const char *src) {
    lexer_t l = lexer_init(src);
    parser_t p = parser_init(l);
    return parse_expr(&p);
}

int test_parse_int() {
    expr_t *e = parse_str("42");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_INT);
    ASSERT_EQ(e->int_val, 42);
    return 1;
}
REGISTER_TEST(test_parse_int);

int test_parse_string() {
    expr_t *e = parse_str("\"hello\"");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_STRING);
    ASSERT_STR_EQ(e->str_val, "hello");
    return 1;
}
REGISTER_TEST(test_parse_string);

int test_parse_nil() {
    expr_t *e = parse_str("nil");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_NIL);
    return 1;
}
REGISTER_TEST(test_parse_nil);

int test_parse_if_no_else() {
    expr_t *e = parse_str("if 1 then 2");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_IF);
    ASSERT(e->if_.cond != NULL);
    ASSERT(e->if_.then != NULL);
    ASSERT(e->if_.else_ == NULL);
    ASSERT_EQ(e->if_.cond->kind, EXPR_INT);
    ASSERT_EQ(e->if_.cond->int_val, 1);
    ASSERT_EQ(e->if_.then->int_val, 2);
    return 1;
}
REGISTER_TEST(test_parse_if_no_else);

int test_parse_if_with_else() {
    expr_t *e = parse_str("if 1 then 2 else 3");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_IF);
    ASSERT(e->if_.else_ != NULL);
    ASSERT_EQ(e->if_.else_->int_val, 3);
    return 1;
}
REGISTER_TEST(test_parse_if_with_else);

int test_parse_while() {
    expr_t *e = parse_str("while 1 do 2");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_WHILE);
    ASSERT(e->while_.cond != NULL);
    ASSERT(e->while_.body != NULL);
    ASSERT_EQ(e->while_.cond->int_val, 1);
    ASSERT_EQ(e->while_.body->int_val, 2);
    return 1;
}
REGISTER_TEST(test_parse_while);

int test_parse_for() {
    expr_t *e = parse_str("for i := 1 to 10 do 0");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_FOR);
    ASSERT_STR_EQ(e->for_.var, "i");
    ASSERT_EQ(e->for_.init->int_val, 1);
    ASSERT_EQ(e->for_.to->int_val, 10);
    ASSERT_EQ(e->for_.body->int_val, 0);
    return 1;
}
REGISTER_TEST(test_parse_for);

int test_parse_assign() {
    expr_t *e = parse_str("x := 42");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_ASSIGN);
    ASSERT_STR_EQ(e->assign.var, "x");
    ASSERT_EQ(e->assign.rhs->int_val, 42);
    return 1;
}
REGISTER_TEST(test_parse_assign);

int test_parse_call_no_args() {
    expr_t *e = parse_str("foo()");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_CALL);
    ASSERT_STR_EQ(e->call.id, "foo");
    ASSERT(e->call.arg_list == NULL);
    return 1;
}
REGISTER_TEST(test_parse_call_no_args);

int test_parse_call_with_args() {
    expr_t *e = parse_str("add(1, 2)");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_CALL);
    ASSERT_STR_EQ(e->call.id, "add");
    ASSERT(e->call.arg_list != NULL);
    ASSERT_EQ(e->call.arg_list->expr->int_val, 1);
    ASSERT_EQ(e->call.arg_list->next->expr->int_val, 2);
    return 1;
}
REGISTER_TEST(test_parse_call_with_args);

int test_parse_ident() {
    expr_t *e = parse_str("x");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_ID);
    ASSERT_STR_EQ(e->id, "x");
    return 1;
}
REGISTER_TEST(test_parse_ident);

int test_parse_let() {
    expr_t *e = parse_str("let var x := 42 in x end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_LET);
    ASSERT(e->let.dec_list != NULL);
    ASSERT_EQ(e->let.dec_list->dec->kind, DEC_VAR);
    ASSERT_STR_EQ(e->let.dec_list->dec->var.id, "x");
    ASSERT_EQ(e->let.dec_list->dec->var.init->int_val, 42);
    ASSERT(e->let.body != NULL);
    return 1;
}
REGISTER_TEST(test_parse_let);

int test_parse_let_multiple_decs() {
    expr_t *e = parse_str("let var x := 1 var y := 2 in x end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_LET);
    ASSERT(e->let.dec_list != NULL);
    ASSERT(e->let.dec_list->next != NULL);
    ASSERT_STR_EQ(e->let.dec_list->dec->var.id, "x");
    ASSERT_STR_EQ(e->let.dec_list->next->dec->var.id, "y");
    return 1;
}
REGISTER_TEST(test_parse_let_multiple_decs);

int test_parse_array_index() {
    expr_t *e = parse_str("a[1]");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_INDEX);
    ASSERT_EQ(e->index_.array->kind, EXPR_ID);
    ASSERT_STR_EQ(e->index_.array->id, "a");
    ASSERT_EQ(e->index_.index->int_val, 1);
    return 1;
}
REGISTER_TEST(test_parse_array_index);

int test_parse_field_access() {
    expr_t *e = parse_str("r.field");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_FIELD);
    ASSERT_EQ(e->field_.record->kind, EXPR_ID);
    ASSERT_STR_EQ(e->field_.record->id, "r");
    ASSERT_STR_EQ(e->field_.field, "field");
    return 1;
}
REGISTER_TEST(test_parse_field_access);

int test_parse_chained_access() {
    expr_t *e = parse_str("a[1].field");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_FIELD);
    ASSERT_EQ(e->field_.record->kind, EXPR_INDEX);
    ASSERT_STR_EQ(e->field_.record->index_.array->id, "a");
    ASSERT_STR_EQ(e->field_.field, "field");
    return 1;
}
REGISTER_TEST(test_parse_chained_access);

int test_parse_nested_index() {
    expr_t *e = parse_str("a[1][2]");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_INDEX);
    ASSERT_EQ(e->index_.array->kind, EXPR_INDEX);
    ASSERT_STR_EQ(e->index_.array->index_.array->id, "a");
    ASSERT_EQ(e->index_.array->index_.index->int_val, 1);
    ASSERT_EQ(e->index_.index->int_val, 2);
    return 1;
}
REGISTER_TEST(test_parse_nested_index);

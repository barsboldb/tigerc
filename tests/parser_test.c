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

int test_binop_add() {
    expr_t *e = parse_str("1 + 2");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.op, OP_ADD);
    ASSERT_EQ(e->binop.left->int_val, 1);
    ASSERT_EQ(e->binop.right->int_val, 2);
    return 1;
}
REGISTER_TEST(test_binop_add);

int test_binop_precedence() {
    // 1 + 2 * 3 should parse as 1 + (2 * 3)
    expr_t *e = parse_str("1 + 2 * 3");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.op, OP_ADD);
    ASSERT_EQ(e->binop.left->int_val, 1);
    ASSERT_EQ(e->binop.right->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.right->binop.op, OP_MUL);
    ASSERT_EQ(e->binop.right->binop.left->int_val, 2);
    ASSERT_EQ(e->binop.right->binop.right->int_val, 3);
    return 1;
}
REGISTER_TEST(test_binop_precedence);

int test_binop_left_assoc() {
    // 1 - 2 - 3 should parse as (1 - 2) - 3
    expr_t *e = parse_str("1 - 2 - 3");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.op, OP_SUB);
    ASSERT_EQ(e->binop.left->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.left->binop.op, OP_SUB);
    ASSERT_EQ(e->binop.left->binop.left->int_val, 1);
    ASSERT_EQ(e->binop.left->binop.right->int_val, 2);
    ASSERT_EQ(e->binop.right->int_val, 3);
    return 1;
}
REGISTER_TEST(test_binop_left_assoc);

int test_binop_comparison() {
    expr_t *e = parse_str("x = 1");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.op, OP_EQ);
    ASSERT_EQ(e->binop.left->kind, EXPR_ID);
    ASSERT_EQ(e->binop.right->int_val, 1);
    return 1;
}
REGISTER_TEST(test_binop_comparison);

int test_binop_logical() {
    // a & b | c should parse as (a & b) | c
    expr_t *e = parse_str("x & y");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.op, OP_AND);
    return 1;
}
REGISTER_TEST(test_binop_logical);

// --- binop edge cases ---

int test_binop_with_idents() {
    expr_t *e = parse_str("x + y");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.op, OP_ADD);
    ASSERT_EQ(e->binop.left->kind, EXPR_ID);
    ASSERT_EQ(e->binop.right->kind, EXPR_ID);
    return 1;
}
REGISTER_TEST(test_binop_with_idents);

int test_binop_call_operand() {
    expr_t *e = parse_str("foo() + 1");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.op, OP_ADD);
    ASSERT_EQ(e->binop.left->kind, EXPR_CALL);
    ASSERT_EQ(e->binop.right->int_val, 1);
    return 1;
}
REGISTER_TEST(test_binop_call_operand);

int test_binop_mixed_precedence() {
    // x + y * z - 1 should parse as (x + (y * z)) - 1
    expr_t *e = parse_str("x + y * z - 1");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.op, OP_SUB);
    ASSERT_EQ(e->binop.left->kind, EXPR_BINOP);
    ASSERT_EQ(e->binop.left->binop.op, OP_ADD);
    ASSERT_EQ(e->binop.right->int_val, 1);
    return 1;
}
REGISTER_TEST(test_binop_mixed_precedence);

// --- if edge cases ---

int test_if_with_binop_cond() {
    expr_t *e = parse_str("if x > 0 then 1 else 0");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_IF);
    ASSERT_EQ(e->if_.cond->kind, EXPR_BINOP);
    ASSERT_EQ(e->if_.cond->binop.op, OP_GT);
    ASSERT_EQ(e->if_.then->int_val, 1);
    ASSERT_EQ(e->if_.else_->int_val, 0);
    return 1;
}
REGISTER_TEST(test_if_with_binop_cond);

int test_nested_if() {
    expr_t *e = parse_str("if 1 then if 2 then 3 else 4");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_IF);
    ASSERT_EQ(e->if_.then->kind, EXPR_IF);
    return 1;
}
REGISTER_TEST(test_nested_if);

// --- while edge cases ---

int test_while_with_binop_cond() {
    expr_t *e = parse_str("while x > 0 do x");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_WHILE);
    ASSERT_EQ(e->while_.cond->kind, EXPR_BINOP);
    ASSERT_EQ(e->while_.cond->binop.op, OP_GT);
    ASSERT_EQ(e->while_.body->kind, EXPR_ID);
    return 1;
}
REGISTER_TEST(test_while_with_binop_cond);

// --- for edge cases ---

int test_for_with_expr_bounds() {
    expr_t *e = parse_str("for i := 0 to n do 0");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_FOR);
    ASSERT_STR_EQ(e->for_.var, "i");
    ASSERT_EQ(e->for_.init->int_val, 0);
    ASSERT_EQ(e->for_.to->kind, EXPR_ID);
    return 1;
}
REGISTER_TEST(test_for_with_expr_bounds);

// --- let edge cases ---

int test_let_var_with_type() {
    expr_t *e = parse_str("let var x: int := 0 in x end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_LET);
    ASSERT_EQ(e->let.dec_list->dec->kind, DEC_VAR);
    ASSERT_STR_EQ(e->let.dec_list->dec->var.type_name, "int");
    ASSERT_EQ(e->let.dec_list->dec->var.init->int_val, 0);
    return 1;
}
REGISTER_TEST(test_let_var_with_type);

int test_let_with_binop_body() {
    expr_t *e = parse_str("let var x := 1 in x + 1 end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_LET);
    ASSERT_EQ(e->let.body->expr->kind, EXPR_BINOP);
    return 1;
}
REGISTER_TEST(test_let_with_binop_body);

int test_let_function_dec() {
    expr_t *e = parse_str("let function add(a: int, b: int): int = a + b in add(1, 2) end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_LET);
    ASSERT_EQ(e->let.dec_list->dec->kind, DEC_FUNC);
    ASSERT_STR_EQ(e->let.dec_list->dec->func.id, "add");
    ASSERT_STR_EQ(e->let.dec_list->dec->func.type_name, "int");
    ASSERT(e->let.dec_list->dec->func.args != NULL);
    return 1;
}
REGISTER_TEST(test_let_function_dec);

int test_let_function_no_return_type() {
    expr_t *e = parse_str("let function greet() = 0 in greet() end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->let.dec_list->dec->kind, DEC_FUNC);
    ASSERT(e->let.dec_list->dec->func.type_name == NULL);
    return 1;
}
REGISTER_TEST(test_let_function_no_return_type);

// --- array index edge cases ---

int test_array_index_with_expr() {
    expr_t *e = parse_str("a[i+1]");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_INDEX);
    ASSERT_EQ(e->index_.index->kind, EXPR_BINOP);
    ASSERT_EQ(e->index_.index->binop.op, OP_ADD);
    return 1;
}
REGISTER_TEST(test_array_index_with_expr);

// --- error cases ---

int test_error_if_missing_then() {
    expr_t *e = parse_str("if 1 2");
    ASSERT(e == NULL);
    return 1;
}
REGISTER_TEST(test_error_if_missing_then);

int test_error_while_missing_do() {
    expr_t *e = parse_str("while 1 2");
    ASSERT(e == NULL);
    return 1;
}
REGISTER_TEST(test_error_while_missing_do);

int test_error_let_missing_end() {
    expr_t *e = parse_str("let var x := 1 in x");
    ASSERT(e == NULL);
    return 1;
}
REGISTER_TEST(test_error_let_missing_end);

int test_error_for_missing_do() {
    expr_t *e = parse_str("for i := 1 to 10 0");
    ASSERT(e == NULL);
    return 1;
}
REGISTER_TEST(test_error_for_missing_do);

// --- expression as initial value in var declaration ---

int test_vardec_expr_init() {
    expr_t *e = parse_str("let var x := 1 + 2 in x end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_LET);
    ASSERT_EQ(e->let.dec_list->dec->var.init->kind, EXPR_BINOP);
    ASSERT_EQ(e->let.dec_list->dec->var.init->binop.op, OP_ADD);
    return 1;
}
REGISTER_TEST(test_vardec_expr_init);

int test_vardec_call_init() {
    expr_t *e = parse_str("let var x := foo() in x end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->let.dec_list->dec->var.init->kind, EXPR_CALL);
    ASSERT_STR_EQ(e->let.dec_list->dec->var.init->call.id, "foo");
    return 1;
}
REGISTER_TEST(test_vardec_call_init);

// --- array index with expression ---

int test_array_index_binop() {
    expr_t *e = parse_str("a[i * 2]");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_INDEX);
    ASSERT_EQ(e->index_.index->kind, EXPR_BINOP);
    ASSERT_EQ(e->index_.index->binop.op, OP_MUL);
    return 1;
}
REGISTER_TEST(test_array_index_binop);

int test_array_index_call() {
    expr_t *e = parse_str("a[foo()]");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_INDEX);
    ASSERT_EQ(e->index_.index->kind, EXPR_CALL);
    return 1;
}
REGISTER_TEST(test_array_index_call);

// --- function call with expression arguments ---

int test_call_expr_args() {
    expr_t *e = parse_str("foo(1 + 2, x * y)");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_CALL);
    ASSERT_EQ(e->call.arg_list->expr->kind, EXPR_BINOP);
    ASSERT_EQ(e->call.arg_list->expr->binop.op, OP_ADD);
    ASSERT_EQ(e->call.arg_list->next->expr->kind, EXPR_BINOP);
    ASSERT_EQ(e->call.arg_list->next->expr->binop.op, OP_MUL);
    return 1;
}
REGISTER_TEST(test_call_expr_args);

int test_call_nested_call_arg() {
    expr_t *e = parse_str("foo(bar(1))");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_CALL);
    ASSERT_EQ(e->call.arg_list->expr->kind, EXPR_CALL);
    ASSERT_STR_EQ(e->call.arg_list->expr->call.id, "bar");
    return 1;
}
REGISTER_TEST(test_call_nested_call_arg);

// --- sequence expressions ---

int test_seq_single() {
    expr_t *e = parse_str("(42)");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_SEQ);
    ASSERT(e->seq != NULL);
    ASSERT_EQ(e->seq->expr->int_val, 42);
    ASSERT(e->seq->next == NULL);
    return 1;
}
REGISTER_TEST(test_seq_single);

int test_seq_multiple() {
    expr_t *e = parse_str("(1; 2; 3)");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_SEQ);
    ASSERT_EQ(e->seq->expr->int_val, 1);
    ASSERT_EQ(e->seq->next->expr->int_val, 2);
    ASSERT_EQ(e->seq->next->next->expr->int_val, 3);
    ASSERT(e->seq->next->next->next == NULL);
    return 1;
}
REGISTER_TEST(test_seq_multiple);

int test_seq_with_exprs() {
    expr_t *e = parse_str("(x := 1; x + 1)");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_SEQ);
    ASSERT_EQ(e->seq->expr->kind, EXPR_ASSIGN);
    ASSERT_EQ(e->seq->next->expr->kind, EXPR_BINOP);
    return 1;
}
REGISTER_TEST(test_seq_with_exprs);

int test_seq_as_while_body() {
    expr_t *e = parse_str("while x > 0 do (x := x - 1; 0)");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_WHILE);
    ASSERT_EQ(e->while_.body->kind, EXPR_SEQ);
    ASSERT_EQ(e->while_.body->seq->expr->kind, EXPR_ASSIGN);
    return 1;
}
REGISTER_TEST(test_seq_as_while_body);

int test_seq_error_trailing_semicolon() {
    expr_t *e = parse_str("(1; 2;)");
    ASSERT(e == NULL);
    return 1;
}
REGISTER_TEST(test_seq_error_trailing_semicolon);

int test_seq_error_unterminated() {
    expr_t *e = parse_str("(1; 2");
    ASSERT(e == NULL);
    return 1;
}
REGISTER_TEST(test_seq_error_unterminated);

// --- record literals ---

int test_record_empty() {
    expr_t *e = parse_str("point {}");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_RECORD);
    ASSERT_STR_EQ(e->record.type_name, "point");
    ASSERT(e->record.fields == NULL);
    return 1;
}
REGISTER_TEST(test_record_empty);

int test_record_single_field() {
    expr_t *e = parse_str("point { x = 1 }");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_RECORD);
    ASSERT_STR_EQ(e->record.type_name, "point");
    ASSERT(e->record.fields != NULL);
    ASSERT_STR_EQ(e->record.fields->name, "x");
    ASSERT_EQ(e->record.fields->val->int_val, 1);
    ASSERT(e->record.fields->next == NULL);
    return 1;
}
REGISTER_TEST(test_record_single_field);

int test_record_multiple_fields() {
    expr_t *e = parse_str("point { x = 3, y = 4 }");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_RECORD);
    ASSERT_STR_EQ(e->record.fields->name, "x");
    ASSERT_EQ(e->record.fields->val->int_val, 3);
    ASSERT(e->record.fields->next != NULL);
    ASSERT_STR_EQ(e->record.fields->next->name, "y");
    ASSERT_EQ(e->record.fields->next->val->int_val, 4);
    return 1;
}
REGISTER_TEST(test_record_multiple_fields);

int test_record_field_expr() {
    expr_t *e = parse_str("point { x = a + 1 }");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_RECORD);
    ASSERT_EQ(e->record.fields->val->kind, EXPR_BINOP);
    ASSERT_EQ(e->record.fields->val->binop.op, OP_ADD);
    return 1;
}
REGISTER_TEST(test_record_field_expr);

int test_record_in_let() {
    expr_t *e = parse_str("let type point = { x : int, y : int } in point { x = 1, y = 2 } end");
    ASSERT(e != NULL);
    ASSERT_EQ(e->kind, EXPR_LET);
    dec_t *d = e->let.dec_list->dec;
    ASSERT_EQ(d->kind, DEC_TYPE);
    ASSERT_STR_EQ(d->type.name, "point");
    ASSERT_EQ(d->type.ty->kind, TY_RECORD);
    ASSERT_STR_EQ(d->type.ty->fields->param->name, "x");
    ASSERT_STR_EQ(d->type.ty->fields->next->param->name, "y");
    expr_t *body = e->let.body->expr;
    ASSERT_EQ(body->kind, EXPR_RECORD);
    ASSERT_STR_EQ(body->record.type_name, "point");
    return 1;
}
REGISTER_TEST(test_record_in_let);

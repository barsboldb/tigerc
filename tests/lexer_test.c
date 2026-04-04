#include "test.h"
#include "lexer.h"
#include <string.h>

int test_single_tokens() {
    lexer_t l = lexer_init("+ - * / = & | ( ) [ ] { } . , ;");
    ASSERT_EQ(next_token(&l).kind, TOK_PLUS);
    ASSERT_EQ(next_token(&l).kind, TOK_MINUS);
    ASSERT_EQ(next_token(&l).kind, TOK_STAR);
    ASSERT_EQ(next_token(&l).kind, TOK_SLASH);
    ASSERT_EQ(next_token(&l).kind, TOK_EQ);
    ASSERT_EQ(next_token(&l).kind, TOK_AND);
    ASSERT_EQ(next_token(&l).kind, TOK_OR);
    ASSERT_EQ(next_token(&l).kind, TOK_LPAREN);
    ASSERT_EQ(next_token(&l).kind, TOK_RPAREN);
    ASSERT_EQ(next_token(&l).kind, TOK_LBRACKET);
    ASSERT_EQ(next_token(&l).kind, TOK_RBRACKET);
    ASSERT_EQ(next_token(&l).kind, TOK_LBRACE);
    ASSERT_EQ(next_token(&l).kind, TOK_RBRACE);
    ASSERT_EQ(next_token(&l).kind, TOK_DOT);
    ASSERT_EQ(next_token(&l).kind, TOK_COMMA);
    ASSERT_EQ(next_token(&l).kind, TOK_SEMICOLON);
    ASSERT_EQ(next_token(&l).kind, TOK_EOF);
    return 1;
}
REGISTER_TEST(test_single_tokens)

int test_multi_char_operators() {
    lexer_t l = lexer_init("<> <= >= := < >");
    ASSERT_EQ(next_token(&l).kind, TOK_NEQ);
    ASSERT_EQ(next_token(&l).kind, TOK_LE);
    ASSERT_EQ(next_token(&l).kind, TOK_GE);
    ASSERT_EQ(next_token(&l).kind, TOK_ASSIGN);
    ASSERT_EQ(next_token(&l).kind, TOK_LT);
    ASSERT_EQ(next_token(&l).kind, TOK_GT);
    ASSERT_EQ(next_token(&l).kind, TOK_EOF);
    return 1;
}
REGISTER_TEST(test_multi_char_operators)

int test_integers() {
    lexer_t l = lexer_init("0 42 123");
    token_t t;
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_INT); ASSERT_EQ(t.int_val, 0);
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_INT); ASSERT_EQ(t.int_val, 42);
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_INT); ASSERT_EQ(t.int_val, 123);
    ASSERT_EQ(next_token(&l).kind, TOK_EOF);
    return 1;
}
REGISTER_TEST(test_integers)

int test_strings() {
    lexer_t l = lexer_init("\"hello\" \"a\\nb\"");
    token_t t;
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_STRING); ASSERT_STR_EQ(t.str_val, "hello");
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_STRING); ASSERT_STR_EQ(t.str_val, "a\nb");
    ASSERT_EQ(next_token(&l).kind, TOK_EOF);
    return 1;
}
REGISTER_TEST(test_strings)

int test_keywords() {
    lexer_t l = lexer_init("if then else while for to do let in end break nil function var type");
    ASSERT_EQ(next_token(&l).kind, TOK_IF);
    ASSERT_EQ(next_token(&l).kind, TOK_THEN);
    ASSERT_EQ(next_token(&l).kind, TOK_ELSE);
    ASSERT_EQ(next_token(&l).kind, TOK_WHILE);
    ASSERT_EQ(next_token(&l).kind, TOK_FOR);
    ASSERT_EQ(next_token(&l).kind, TOK_TO);
    ASSERT_EQ(next_token(&l).kind, TOK_DO);
    ASSERT_EQ(next_token(&l).kind, TOK_LET);
    ASSERT_EQ(next_token(&l).kind, TOK_IN);
    ASSERT_EQ(next_token(&l).kind, TOK_END);
    ASSERT_EQ(next_token(&l).kind, TOK_BREAK);
    ASSERT_EQ(next_token(&l).kind, TOK_NIL);
    ASSERT_EQ(next_token(&l).kind, TOK_FUNCTION);
    ASSERT_EQ(next_token(&l).kind, TOK_VAR);
    ASSERT_EQ(next_token(&l).kind, TOK_TYPE);
    ASSERT_EQ(next_token(&l).kind, TOK_EOF);
    return 1;
}
REGISTER_TEST(test_keywords)

int test_identifiers() {
    lexer_t l = lexer_init("foo bar _x x1");
    token_t t;
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_ID); ASSERT_STR_EQ(t.str_val, "foo");
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_ID); ASSERT_STR_EQ(t.str_val, "bar");
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_ID); ASSERT_STR_EQ(t.str_val, "_x");
    t = next_token(&l); ASSERT_EQ(t.kind, TOK_ID); ASSERT_STR_EQ(t.str_val, "x1");
    ASSERT_EQ(next_token(&l).kind, TOK_EOF);
    return 1;
}
REGISTER_TEST(test_identifiers)

int test_comments() {
    lexer_t l = lexer_init("/* comment */ 42");
    token_t t = next_token(&l);
    ASSERT_EQ(t.kind, TOK_INT);
    ASSERT_EQ(t.int_val, 42);
    return 1;
}
REGISTER_TEST(test_comments)

int test_nested_comments() {
    lexer_t l = lexer_init("/* outer /* inner */ still outer */ 1");
    token_t t = next_token(&l);
    ASSERT_EQ(t.kind, TOK_INT);
    ASSERT_EQ(t.int_val, 1);
    return 1;
}
REGISTER_TEST(test_nested_comments)

int test_unterminated_comment() {
    lexer_t l = lexer_init("/* never closed");
    ASSERT_EQ(next_token(&l).kind, TOK_ERROR);
    return 1;
}
REGISTER_TEST(test_unterminated_comment)

int test_unterminated_string() {
    lexer_t l = lexer_init("\"never closed");
    ASSERT_EQ(next_token(&l).kind, TOK_ERROR);
    return 1;
}
REGISTER_TEST(test_unterminated_string)

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "ast.h"

static expr_t *parse_primary(parser_t *p);

parser_t parser_init(lexer_t lexer) {
  parser_t p;
  p.lexer   = lexer;
  p.current = next_token(&p.lexer);
  p.next    = next_token(&p.lexer);
  return p;
}

static int get_infix_bp(token_kind_t kind) {
  switch (kind) {
    case TOK_OR:    return 2;
    case TOK_AND:   return 4;
    case TOK_EQ:
    case TOK_NEQ:
    case TOK_LT:
    case TOK_GT:
    case TOK_LE:
    case TOK_GE:    return 6;
    case TOK_PLUS:
    case TOK_MINUS: return 8;
    case TOK_STAR:
    case TOK_SLASH: return 10;
    default:        return 0;
  }
}

static binop_t get_binop_kind(token_kind_t kind) {
  switch (kind) {
    case TOK_OR:    return OP_OR;
    case TOK_AND:   return OP_AND;
    case TOK_EQ:    return OP_EQ;
    case TOK_NEQ:   return OP_NEQ;
    case TOK_LT:    return OP_LT;
    case TOK_GT:    return OP_GT;
    case TOK_LE:    return OP_LE;
    case TOK_GE:    return OP_GE;
    case TOK_PLUS:  return OP_ADD;
    case TOK_MINUS: return OP_SUB;
    case TOK_STAR:  return OP_MUL;
    case TOK_SLASH: return OP_DIV;
    default:        return -1;
  }
}

static token_t advance(parser_t *parser) {
  token_t old = parser->current;
  parser->current = parser->next;
  parser->next    = next_token(&parser->lexer);
  return old;
}

static int expect(parser_t *parser, token_kind_t type) {
  if (parser->current.kind == type) {
    advance(parser);
    return 0;
  }
  fprintf(stderr, "error: %d:%d: expected %s but got %s\n",
      parser->current.line,
      parser->current.col,
      token_kind_name(type),
      token_kind_name(parser->current.kind));
  return -1;
}

static expr_t *make_expr(expr_kind_t kind, int line, int col) {
  expr_t *e = malloc(sizeof(expr_t));
  e->kind = kind;
  e->line = line;
  e->col  = col;

  if (kind == EXPR_LET) {
    e->let.body = NULL;
    e->let.dec_list = NULL;
  }

  return e;
}

static dec_t *make_dec(dec_kind_t kind) {
  dec_t *d = malloc(sizeof(dec_t));
  d->kind = kind;
  if (kind == DEC_FUNC) {
    d->func.args = NULL;
  }
  return d;
}

static expr_t *make_binop(binop_t kind, expr_t *lhs, expr_t *rhs, int line, int col) {
  expr_t *e = make_expr(EXPR_BINOP, line, col);
  e->binop.left = lhs;
  e->binop.right = rhs;
  e->binop.op = kind;
  return e;
}

static expr_t *parse_int(parser_t *p) {
  expr_t *e = make_expr(EXPR_INT, p->current.line, p->current.col);
  e->int_val = p->current.int_val;
  advance(p);
  return e;
}

static expr_t *parse_string(parser_t *p) {
  expr_t *e = make_expr(EXPR_STRING, p->current.line, p->current.col);
  e->str_val = p->current.str_val;
  advance(p);
  return e;
}

static expr_t *parse_nil(parser_t *p) {
  expr_t *e = make_expr(EXPR_NIL, p->current.line, p->current.col);
  advance(p);
  return e;
}

static expr_t *parse_if(parser_t *p) {
  expr_t *e = make_expr(EXPR_IF, p->current.line, p->current.col);
  advance(p);
  e->if_.cond = parse_expr(p);

  if (expect(p, TOK_THEN) < 0) return NULL;
  e->if_.then = parse_expr(p);

  if (p->current.kind == TOK_ELSE) {
    advance(p);
    e->if_.else_ = parse_expr(p);
  } else {
    e->if_.else_ = NULL;
  }

  return e;
}

static expr_t *parse_while(parser_t *p) {
  expr_t *e = make_expr(EXPR_WHILE, p->current.line, p->current.col);

  advance(p);
  e->while_.cond = parse_expr(p);

  if (expect(p, TOK_DO) < 0) return NULL;
  e->while_.body = parse_expr(p);

  return e;
}

static expr_t *parse_for(parser_t *p) {
  expr_t *e = make_expr(EXPR_FOR, p->current.line, p->current.col);
  advance(p);
  token_t current = p->current;
  if (expect(p, TOK_ID) < 0) return NULL;
  e->for_.var = current.str_val;

  if (expect(p, TOK_ASSIGN) < 0) return NULL;
  e->for_.init = parse_expr(p);
  if (expect(p, TOK_TO) < 0) return NULL;
  e->for_.to = parse_expr(p);
  if (expect(p, TOK_DO) < 0) return NULL;
  e->for_.body = parse_expr(p);

  return e;
}

static dec_list_t *declist_insert(dec_list_t *list, dec_t *dec) {
  dec_list_t *last = list; 

  if (list == NULL) {
    last = malloc(sizeof(dec_list_t));
    last->next = NULL;
    last->dec = dec;
    return last;
  }

  while (last->next != NULL)
    last = last->next;

  last->next = malloc(sizeof(dec_list_t));
  last->next->dec = dec;
  last->next->next = NULL;
  return list;
} 

static param_list_t *paramlist_insert(param_list_t *list, param_t *param) {
  param_list_t *last = list;

  if (list == NULL) {
    last = malloc(sizeof(param_list_t));
    last->param = param;
    last->next = NULL;
    return last;
  }

  while (last->next != NULL)
    last = last->next;

  last->next = malloc(sizeof(param_list_t));
  last->next->param = param;
  last->next->next = NULL;
  return list;
}

static expr_list_t *exprlist_insert(expr_list_t *list, expr_t *e) {
  if (list == NULL) {
    expr_list_t *el = malloc(sizeof(expr_list_t));
    el->expr = e;
    el->next = NULL;
    return el;
  }

  expr_list_t *last = list;
  while (last->next) {
    last = last->next;
  }
  last->next = malloc(sizeof(expr_list_t));
  last->next->expr = e;
  last->next->next = NULL;

  return list;
}

static field_list_t *fieldlist_insert(field_list_t *list, char *name, expr_t *val) {
  if (!list) {
    field_list_t *head = malloc(sizeof(field_list_t));
    head->name = name;
    head->val  = val;
    return head;
  }
  
  field_list_t *last = list;
  while (last->next) {
    last = last->next;
  }

  last->next = malloc(sizeof(field_list_t));
  last->next->name = name;
  last->next->val  = val;
  last->next->next = NULL;
  return list;
}

static dec_t *parse_vardec(parser_t *p) {
  if (expect(p, TOK_VAR) < 0) return NULL;
  token_t c = p->current;

  if (expect(p, TOK_ID) < 0) return NULL;
  dec_t *d = make_dec(DEC_VAR);
  d->var.id = c.str_val;

  if (p->current.kind == TOK_COLON) {
    advance(p);
    c = p->current;
    if (expect(p, TOK_ID) < 0) {
      free(d);
      return NULL;
    }
    d->var.type_name = c.str_val;
  }
  
  if (expect(p, TOK_ASSIGN) < 0) {
    free(d);
    return NULL;
  }

  d->var.init = parse_expr(p);
  return d;
}

static dec_t *parse_funcdec(parser_t *p) {
  if (expect(p, TOK_FUNCTION) < 0) return NULL;
  token_t c = p->current;

  if (expect(p, TOK_ID) < 0) return NULL;
  dec_t *d = make_dec(DEC_FUNC);
  d->func.id = c.str_val;

  if (expect(p, TOK_LPAREN) < 0) return NULL;
  while (p->current.kind != TOK_RPAREN) {
    if (p->current.kind == TOK_EOF) return NULL;

    param_t *param = malloc(sizeof(param_t));
    token_t name_tok = p->current;
    if (expect(p, TOK_ID) < 0) return NULL;
    param->name = name_tok.str_val;
    if (expect(p, TOK_COLON) < 0) return NULL;
    c = p->current;
    if (expect(p, TOK_ID) < 0) return NULL;
    param->type_name = c.str_val;
    d->func.args = paramlist_insert(d->func.args, param);
    if (p->current.kind == TOK_COMMA) advance(p);
  }

  if (expect(p, TOK_RPAREN) < 0) return NULL;
  if (p->current.kind == TOK_COLON) {
    advance(p);
    c = p->current;
    if (expect(p, TOK_ID) < 0) return NULL;
    d->func.type_name = c.str_val;
  }

  if (expect(p, TOK_EQ) < 0) return NULL;
  d->func.body = parse_expr(p);
  return d;
}

static dec_t *parse_typedec(parser_t *p) {
  if (expect(p, TOK_TYPE) < 0) return NULL;
  token_t current = p->current;
  if (expect(p, TOK_ID) < 0) return NULL;
  dec_t *d = make_dec(DEC_TYPE);
  ty_t  *ty = malloc(sizeof(ty_t));
  d->type.name = current.str_val;

  if (expect(p, TOK_EQ) < 0) { free(d); free(ty); return NULL; }

  if (p->current.kind == TOK_LBRACE) {
    ty->kind = TY_RECORD;
    advance(p);
    while (p->current.kind != TOK_RBRACE) {
      param_t *param = malloc(sizeof(param_t));
      param->name = p->current.str_val;
      if (expect(p, TOK_ID) < 0) return NULL;
      if (expect(p, TOK_COLON) < 0) return NULL;
      param->type_name = p->current.str_val;
      if (expect(p, TOK_ID) < 0) return NULL;
      ty->fields = paramlist_insert(ty->fields, param);
      if (p->current.kind == TOK_COMMA) {
        advance(p);
      }
    }

    if (expect(p, TOK_RBRACE) < 0) return NULL;
  } else if (p->current.kind == TOK_ARRAY) {
    advance(p);
    if (expect(p, TOK_OF) < 0) {
      free(ty);
      free(d);
      return NULL;
    }
    ty->kind = TY_ARRAY;
    ty->array_of = p->current.str_val;
    if (expect(p, TOK_ID) < 0) {
      free(ty);
      free(d);
      return NULL;
    }
  } else {
    ty->kind = TY_NAME;
    ty->alias = p->current.str_val;
    if (expect(p, TOK_ID) < 0) { free(ty); free(d); return NULL; }
  }

  d->type.ty = ty;

  return d;
}

static expr_t *parse_let(parser_t *p) {
  expr_t *e = make_expr(EXPR_LET, p->current.line, p->current.col);
  dec_list_t *declist = NULL;
  advance(p);
  
  while (p->current.kind != TOK_IN) {
    dec_t *d;
    if (p->current.kind == TOK_VAR) {
      d = parse_vardec(p);
      declist = declist_insert(declist, d);
    } else if (p->current.kind == TOK_FUNCTION) {
      d = parse_funcdec(p);
      declist = declist_insert(declist, d);
    } else if (p->current.kind == TOK_TYPE) {
      d = parse_typedec(p);
      declist = declist_insert(declist, d);
    } else {
      fprintf(stderr, "error: unexpected token in let declarations\n");
      return NULL;
    }
  }

  e->let.dec_list = declist;

  if (expect(p, TOK_IN) < 0) { free(e); return NULL; }

  while (p->current.kind != TOK_END) {
    if (p->current.kind == TOK_EOF) {
      fprintf(stderr, "unterminated let expression\n");
      return NULL;
    }

    expr_t *bodyexpr = parse_expr(p);
    e->let.body = exprlist_insert(e->let.body, bodyexpr);
    if (p->current.kind == TOK_SEMICOLON) {
      advance(p);
    }
  }

  if (expect(p, TOK_END) < 0) return NULL;

  return e;
}

static expr_t *parse_assign(parser_t *p) {
  expr_t *e = make_expr(EXPR_ASSIGN, p->current.line, p->current.col);
  e->assign.var = p->current.str_val;
  if (expect(p, TOK_ID) < 0) { return NULL; }
  if (expect(p, TOK_ASSIGN) < 0) { return NULL; }
  e->assign.rhs = parse_expr(p);

  return e;
}

static expr_t *parse_call(parser_t *p) {
  expr_t *e = make_expr(EXPR_CALL, p->current.line, p->current.col);
  e->call.id = p->current.str_val;
  if (expect(p, TOK_ID) < 0) { return NULL; }
  if (expect(p, TOK_LPAREN) < 0) { return NULL; }
  e->call.arg_list = NULL;
  while (p->current.kind != TOK_RPAREN) {
    if (p->current.kind == TOK_EOF) return NULL;
    expr_t *arg = parse_expr(p);
    e->call.arg_list = exprlist_insert(e->call.arg_list, arg);
    if (p->current.kind == TOK_COMMA) advance(p);
  }
  if (expect(p, TOK_RPAREN) < 0) {
    return NULL;
  }
  return e;
}

static expr_t *parse_lvalue(parser_t *p) {
  expr_t *e = make_expr(EXPR_ID, p->current.line, p->current.col);
  e->id = p->current.str_val;
  advance(p);

  while (p->current.kind == TOK_DOT || p->current.kind == TOK_LBRACKET) {
    if (p->current.kind == TOK_DOT) {
      advance(p);
      expr_t *field = make_expr(EXPR_FIELD, p->current.line, p->current.col);
      field->field_.record = e;
      field->field_.field  = p->current.str_val;
      if (expect(p, TOK_ID) < 0) return NULL;
      e = field;
    } else {
      advance(p);
      expr_t *idx = make_expr(EXPR_INDEX, p->current.line, p->current.col);
      idx->index_.array = e;
      idx->index_.index = parse_expr(p);
      if (expect(p, TOK_RBRACKET) < 0) return NULL;
      e = idx;
    }
  }
  return e;
}

expr_list_t *exprseq_insert(expr_list_t *head, expr_t *n) {
  if (!n) return head;

  if (!head) {
    expr_list_t *list = malloc(sizeof(expr_list_t));
    list->expr = n;
    list->next = NULL;
    return list;
  }

  expr_list_t *last = head;

  while (last->next) {
    last = last->next;
  }

  last->next = malloc(sizeof(expr_list_t));
  last->next->expr = n;
  last->next->next = NULL;

  return head;
}

static expr_t *parse_seq(parser_t *p) {
  expr_t *e = make_expr(EXPR_SEQ, p->current.line, p->current.col);
  e->seq = NULL;
  advance(p);

  while (p->current.kind != TOK_RPAREN) {
    if (p->current.kind == TOK_EOF) {
      fprintf(stderr, "error: %d:%d: unterminated expression sequence\n",
          e->line, e->col);
      return NULL;
    }

    expr_t *node = parse_expr(p);
    e->seq = exprseq_insert(e->seq, node);

    if (p->current.kind == TOK_SEMICOLON) {
      advance(p);
      if (p->current.kind == TOK_RPAREN) {
        fprintf(stderr, "error: %d:%d: trailing semicolon in sequence\n",
            e->line, e->col);
        return NULL;
      }
    }
  }

  if (expect(p, TOK_RPAREN) < 0) return NULL;
  return e;
}

static expr_t *parse_expr_bp(parser_t *p, int min_bp) {
  expr_t *lhs = parse_primary(p);

  while (1) {
    int bp = get_infix_bp(p->current.kind);
    if (bp <= min_bp) break;

    token_t op = advance(p);
    expr_t *rhs = parse_expr_bp(p, bp + 1);
    lhs = make_binop(get_binop_kind(op.kind), lhs, rhs, op.line, op.col);
  }

  return lhs;
}

static expr_t *parse_record(parser_t *p) {
  expr_t *r = make_expr(EXPR_RECORD, p->current.line, p->current.col);
  r->record.type_name = p->current.str_val;
  advance(p);
  if (expect(p, TOK_LBRACE) < 0) return NULL;

  field_list_t *fields = NULL;

  while (p->current.kind != TOK_RBRACE) {
    char *name = p->current.str_val;
    if (expect(p, TOK_ID) < 0) return NULL;
    if (expect(p, TOK_EQ) < 0) return NULL;
    expr_t *val = parse_expr(p);
    
    fields = fieldlist_insert(fields, name, val);
    if (p->current.kind == TOK_COMMA) advance(p);
  }

  r->record.fields = fields;

  if (expect(p, TOK_RBRACE) < 0) return NULL;

  return r;
}

static expr_t *parse_primary(parser_t *p) {
  switch (p->current.kind) {
    case TOK_INT:    return parse_int(p);
    case TOK_STRING: return parse_string(p);
    case TOK_NIL:    return parse_nil(p);
    case TOK_IF:     return parse_if(p);
    case TOK_WHILE:  return parse_while(p);
    case TOK_FOR:    return parse_for(p);
    case TOK_LET:    return parse_let(p);
    case TOK_LPAREN: return parse_seq(p);
    case TOK_MINUS: {
      token_t tok = advance(p);
      expr_t *operand = parse_primary(p);
      expr_t *zero = make_expr(EXPR_INT, tok.line, tok.col);
      zero->int_val = 0;
      return make_binop(OP_SUB, zero, operand, tok.line, tok.col);
    }
    case TOK_ID:
      if (p->next.kind == TOK_ASSIGN)   return parse_assign(p);
      if (p->next.kind == TOK_LPAREN)   return parse_call(p);
      if (p->next.kind == TOK_LBRACE)   return parse_record(p);
      return parse_lvalue(p);
    default: return NULL;
  }
}

expr_t *parse_expr(parser_t *p) {
  return parse_expr_bp(p, 0);
}


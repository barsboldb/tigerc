#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semant.h"

symtab_t *venv;
symtab_t *tenv;

static int current_depth = 0;

static env_entry_t *make_func_entry(param_ty_t *params, semty_t *ret, int depth) {
  env_entry_t *e = malloc(sizeof(env_entry_t));
  e->kind        = ENV_FUNC;
  e->func.params = params;
  e->func.ret    = ret;
  e->func.depth  = depth;
  return e;
}

static param_ty_t *make_param_ty(semty_t *type, param_ty_t *next) {
  param_ty_t *p = malloc(sizeof(param_ty_t));
  p->type = type;
  p->next = next;
  return p;
}

symtab_t *semant_base_tenv() {
  tenv = symtab_new(64);

  semty_t *int_ty    = malloc(sizeof(semty_t));
  semty_t *string_ty = malloc(sizeof(semty_t));
  semty_t *null_ty   = malloc(sizeof(semty_t));
  int_ty->kind    = SEMTY_INT;
  string_ty->kind = SEMTY_STRING;
  null_ty->kind   = SEMTY_NIL;
  symtab_enter_scope(tenv);
  symtab_insert(tenv, "int",    int_ty);
  symtab_insert(tenv, "string", string_ty);
  symtab_insert(tenv, "nil",   null_ty);

  return tenv;
}
symtab_t *semant_base_venv(symtab_t *tenv) {
  venv = symtab_new(64);

  semty_t *ty_int    = symtab_lookup(tenv, "int");
  semty_t *ty_string = symtab_lookup(tenv, "string");
  semty_t *ty_void   = malloc(sizeof(semty_t));
  ty_void->kind = SEMTY_VOID;

  symtab_enter_scope(venv);
  symtab_insert(venv, "print",
    make_func_entry(
      make_param_ty(ty_string, NULL),
      ty_void,
      0
    )
  );
  symtab_insert(venv, "flush", make_func_entry(NULL, ty_void, 0));
  symtab_insert(venv, "getchar", make_func_entry(NULL, ty_string, 0));
  symtab_insert(venv, "ord",
    make_func_entry(
      make_param_ty(ty_string, NULL),
      ty_int, 0
    )
  );
  symtab_insert(venv, "chr",
    make_func_entry(
      make_param_ty(ty_int, NULL),
      ty_string, 0
    )
  );
  symtab_insert(venv, "size",
    make_func_entry(
      make_param_ty(ty_string, NULL),
      ty_int, 0
    )
  );
  symtab_insert(venv, "not",
    make_func_entry(
      make_param_ty(ty_int, NULL),
      ty_int, 0
    )
  );
  symtab_insert(venv, "exit",
    make_func_entry(
      make_param_ty(ty_int, NULL),
      ty_void, 0
    )
  );
  symtab_insert(venv, "concat",
    make_func_entry(
      make_param_ty(ty_string,
        make_param_ty(ty_string, NULL)),
      ty_string, 0
    )
  );
  symtab_insert(venv, "substring",
    make_func_entry(
      make_param_ty(ty_string,
        make_param_ty(ty_int,
          make_param_ty(ty_int, NULL)
        )
      ),
      ty_string, 0
    )
  );

  return venv;
}

static semty_t *actual_ty(symtab_t *tenv, semty_t *ty) {
  while (ty && ty->kind == SEMTY_NAME)
    ty = symtab_lookup(tenv, ty->name);
  return ty;
}

semty_t *trans_ty(ty_t *ty) {
  switch (ty->kind) {
    case TY_NAME:
      return (semty_t *)symtab_lookup(tenv, ty->alias);
    case TY_ARRAY: {
      semty_t *s = malloc(sizeof(semty_t));
      s->kind = SEMTY_ARRAY;
      s->array = (semty_t *)symtab_lookup(tenv, ty->array_of);
      return s;
    }
    case TY_RECORD: {
      semty_t *s = malloc(sizeof(semty_t));
      field_ty_t *fields = NULL, *last = NULL;
      param_list_t *p = ty->fields;
      while (p) {
        field_ty_t *f = malloc(sizeof(field_ty_t));
        f->name = p->param->name;
        f->type = (semty_t *)symtab_lookup(tenv, p->param->type_name);
        f->next = NULL;
        if (!fields) fields = f;
        else last->next = f;
        last = f;
        p = p->next;
      }
      s->kind = SEMTY_RECORD;
      s->record = fields;
      return s;
    }
  };
}

semty_t *trans_var(expr_t *e) {
  switch (e->kind) {
    case EXPR_ID: {
      env_entry_t *entry = symtab_lookup(venv, e->id);
      if (!entry) {
        fprintf(stderr, "error: undefined variable\n");
        return NULL;
      }
      if (entry->kind != ENV_VAR) {
        fprintf(stderr, "error: cannot use function as a variable\n");
        return NULL;
      }
      e->ty = entry->var;
      return entry->var;
    }
    case EXPR_FIELD: {
      semty_t *record_ty = trans_var(e->field_.record);
      if (record_ty->kind != SEMTY_RECORD) {
        fprintf(stderr, "error: field access is only allowed with record type\n");
        return NULL;
      }
      field_ty_t *field_ty = record_ty->record;

      while (field_ty) {
        if (strcmp(field_ty->name, e->field_.field) == 0) break;
        field_ty = field_ty->next;
      }

      if (!field_ty) {
        fprintf(stderr, "error: unknown field '%s'\n", e->field_.field);
        return NULL;
      }
      e->ty = field_ty->type;
      return field_ty->type;
    }
    case EXPR_INDEX: {
      semty_t *array_ty = trans_var(e->index_.array);
      if (array_ty->kind != SEMTY_ARRAY) {
        fprintf(stderr, "error: cannot access index of non-array variable\n");
        return NULL;
      }
      semty_t *index_ty = trans_expr(e->index_.index);
      if (index_ty->kind != SEMTY_INT) {
        fprintf(stderr, "error: cannot access non-integer array index\n");
        return NULL;
      }
      e->ty = array_ty->array;
      return array_ty->array;
    }
    default: return NULL;
  }
}

void trans_dec_header(dec_t *dec) {
  switch (dec->kind) {
    case DEC_FUNC: {
      env_entry_t *entry = malloc(sizeof(env_entry_t));
      entry->kind = ENV_FUNC;
      param_list_t *p = dec->func.args;
      param_ty_t *p_ty = NULL, *it = NULL;
      while (p) {
        param_ty_t *node = malloc(sizeof(param_ty_t));
        node->type = symtab_lookup(tenv, p->param->type_name);
        node->next = NULL;
        if (!p_ty) p_ty = it = node;
        else {
          it = it->next = node;
        }
        p = p->next;
      }
      entry->func.params = p_ty;
      entry->func.depth  = current_depth + 1;
      entry->func.ret    = dec->func.type_name ? symtab_lookup(tenv, dec->func.type_name) : NULL;
      symtab_insert(venv, dec->func.id, entry);
      return;
    }
    case DEC_TYPE: {
      semty_t *ty = malloc(sizeof(semty_t));
      ty->kind = SEMTY_NAME;
      ty->name = dec->type.name;
      symtab_insert(tenv, dec->type.name, ty);
    }
    default: return;
  }
}

semty_t *trans_expr(expr_t *e) {
  semty_t *s = malloc(sizeof(semty_t));
  switch (e->kind) {
    case EXPR_INT:
      s->kind = SEMTY_INT;
      e->ty = s;
      return s;
    case EXPR_STRING:
      s->kind = SEMTY_STRING;
      e->ty = s;
      return s;
    case EXPR_NIL:
      s->kind = SEMTY_NIL;
      e->ty = s;
      return s;
    case EXPR_ID:
    case EXPR_FIELD:
    case EXPR_INDEX:
      s = trans_var(e);
      e->ty = s;
      return s;
    case EXPR_ASSIGN: {
      semty_t *rhs = trans_var(e->assign.lhs);
      semty_t *lhs = trans_expr(e->assign.rhs);
      s->kind = SEMTY_VOID;
      if (!lhs || !rhs) return NULL;
      s->kind = SEMTY_VOID;
      if (rhs->kind != SEMTY_NIL && lhs->kind == SEMTY_RECORD) {
        e->ty = s;
        return s;
      }
      if (lhs->kind != rhs->kind) {
        fprintf(stderr, "error: type mismatch in assignment\n");
      }
      e->ty = s;
      return s;
    }
    case EXPR_IF: {
      semty_t *cond = trans_expr(e->if_.cond);
      if (cond->kind != SEMTY_INT) {
        fprintf(stderr, "error: if expr condition must be int\n");
        return NULL;
      }
      semty_t *then = trans_expr(e->if_.then);
      if (!e->if_.else_) {
        return then;
      }
      semty_t *else_ = trans_expr(e->if_.else_);
      if (!else_ && !then) {
        fprintf(stderr, "error: cannot infer return type, add a return type annotation");
        return NULL;
      }
      if (!else_) {
        return then;
      }
      if (then->kind != else_->kind) {
        fprintf(stderr, "error: if expr type is ambigious\n");
        return NULL;
      }
      e->ty = then;
      return then;
    }
    case EXPR_WHILE: {
      semty_t *cond = trans_expr(e->while_.cond);
      if (cond->kind != SEMTY_INT) {
        fprintf(stderr, "error: while expr condition must be int\n");
        return NULL;
      }
      trans_expr(e->while_.body);
      s->kind = SEMTY_VOID;
      e->ty = s;
      return s;
    }
    case EXPR_FOR: {
      semty_t *init = trans_expr(e->for_.init);
      semty_t *to   = trans_expr(e->for_.to);
      if (init->kind != to->kind) {
        fprintf(stderr, "error: for expr type mismatch\n");
        return NULL;
      }
      if (init->kind != SEMTY_INT) {
        fprintf(stderr, "error: for expr iterator type should be int\n");
        return NULL;
      }
      s->kind = SEMTY_VOID;
      e->ty = s;
      return s;
    }
    case EXPR_CALL: {
      env_entry_t *f = symtab_lookup(venv, e->call.id);
      if (!f) {
        fprintf(stderr, "error: undefined function '%s'\n", e->call.id);
        return NULL;
      }
      if (f->kind == ENV_VAR) {
        fprintf(stderr, "error: cannot invoke variable call\n");
        return NULL;
      }

      param_ty_t *p = f->func.params;
      expr_list_t *a = e->call.arg_list;
      while (p) {
        semty_t *arg_ty  = trans_expr(a->expr);
        semty_t *param_ty = actual_ty(tenv, p->type);
        if (param_ty->kind != arg_ty->kind) {
          if (!(param_ty->kind == SEMTY_RECORD && arg_ty->kind == SEMTY_NIL)) {
            fprintf(stderr, "error: param type mismatch\n");
            return NULL;
          }
        }
        p = p->next;
        a = a->next;
      }
      e->ty = f->func.ret;
      e->call.callee_depth = f->func.depth;
      return f->func.ret;
    }
    case EXPR_SEQ: {
      expr_list_t *seq = e->seq;
      while (seq->next) {
        trans_expr(seq->expr);
        seq = seq->next;
      }
      semty_t *ty = trans_expr(seq->expr);
      e->ty = ty;
      return ty;
    }
    case EXPR_LET: {
      symtab_enter_scope(venv);
      symtab_enter_scope(tenv);
      dec_list_t *d = e->let.dec_list;
      while (d) {
        trans_dec_header(d->dec);
        d = d->next;
      }
      d = e->let.dec_list;
      while (d) {
        trans_dec(d->dec);
        d = d->next;
      }
      semty_t *result = NULL;
      expr_list_t *l = e->let.body;
      while (l) {
        result = trans_expr(l->expr);
        l = l->next;
      }
      symtab_exit_scope(venv);
      symtab_exit_scope(tenv);
      e->ty = result;
      return result;
    }
    case EXPR_BINOP: {
      semty_t *l = trans_expr(e->binop.left);
      semty_t *r = trans_expr(e->binop.right);
      semty_t *res = malloc(sizeof(semty_t));

      switch (e->binop.op) {
        case OP_ADD:
        case OP_SUB:
        case OP_DIV:
        case OP_MUL:
        case OP_GT:
        case OP_GE:
        case OP_LT:
        case OP_LE:
        case OP_AND:
        case OP_OR:
          if (!l) {
            fprintf(stderr, "error: unknown types around binary operation\n");
            return NULL;
          }
          if (l->kind != SEMTY_INT) {
            fprintf(stderr, "error: operands should be ints\n");
            return NULL;
          }
          if (r->kind != SEMTY_INT) {
            fprintf(stderr, "error: operands should be ints\n");
            return NULL;
          }
          res->kind = SEMTY_INT;
          e->ty = res;
          return res;
        case OP_EQ:
        case OP_NEQ:
          l = actual_ty(tenv, l);
          r = actual_ty(tenv, r);
          if (l->kind != r->kind) {
            fprintf(stderr, "error: operands should be same type\n");
            return NULL;
          }
          res->kind = SEMTY_INT;
          e->ty = res;
          return res;
      }
    }
    case EXPR_RECORD: {
      semty_t *t = symtab_lookup(tenv, e->record.type_name);
      if (t->kind != SEMTY_RECORD) {
        fprintf(stderr, "error: record type expected\n");
      }
      field_ty_t *field_ty = NULL;
      field_list_t *field = e->record.fields;
      while (field) {
        field_ty = t->record;
        while (field_ty) {
          if (strcmp(field_ty->name, field->name) == 0) break;
          field_ty = field_ty->next;
        }

        semty_t *s = trans_expr(field->val);

        if (s->kind == SEMTY_NIL) {
          field = field->next;
          continue;
        }
        
        s = actual_ty(tenv, s);
        semty_t *f = actual_ty(tenv, field_ty->type);
        if (s->kind != f->kind) {
          fprintf(stderr, "error: record field type mismatch\n");
        }
        field = field->next;
      }
      e->ty = t;
      return t;
    }
    case EXPR_ARRAY: {
      semty_t *semty = symtab_lookup(tenv, e->array.type_name);
      semty_t *size  = trans_expr(e->array.size);
      semty_t *init  = trans_expr(e->array.init);

      if (semty->kind != SEMTY_ARRAY) {
        fprintf(stderr, "error: array type expected\n");
      }
      if (size->kind != SEMTY_INT) {
        fprintf(stderr, "error: array size must be int\n");
      }
      if (init->kind != semty->array->kind) {
        fprintf(stderr, "error: array init type mismatch\n");
      }
      
      e->ty = semty;
      return semty;
    }
  }
}

void trans_dec(dec_t *dec) {
  switch (dec->kind) {
    case DEC_FUNC: {
      env_entry_t *entry = symtab_lookup(venv, dec->func.id);
      symtab_enter_scope(venv);

      param_list_t *arg = dec->func.args;
      param_ty_t   *pty = entry->func.params;
      while (arg) {
        env_entry_t *pe = malloc(sizeof(env_entry_t));
        pe->kind = ENV_VAR;
        pe->var  = pty->type;
        symtab_insert(venv, arg->param->name, pe);
        arg = arg->next;
        pty = pty->next;
      }

      current_depth++;
      semty_t *body_ty = dec->func.body ? trans_expr(dec->func.body) : NULL;
      symtab_exit_scope(venv);

      if (entry->func.ret) {
        if (body_ty && body_ty->kind != entry->func.ret->kind) {
          fprintf(stderr, "error function '%s' body type does not match declared return type\n", dec->func.id);
        }
      } else {
        if (!body_ty) {
          fprintf(stderr, "error: cannot infer return type of '%s', add a return type annotation\n", dec->func.id);
        }
        entry->func.ret = body_ty;
      }
      current_depth--;
      return;
    }
    case DEC_VAR: {
      env_entry_t *s = malloc(sizeof(env_entry_t));
      s->kind = ENV_VAR;

      semty_t *init_ty = trans_expr(dec->var.init);
      if (dec->var.type_name) {
        semty_t *declared = symtab_lookup(tenv, dec->var.type_name);
        if (declared->kind != init_ty->kind) {
          fprintf(stderr, "error: type mismatch in var declaration '%s'\n", dec->var.id);
        }
        s->var = declared;
      }
      else s->var = init_ty;
      symtab_insert(venv, dec->var.id, s);
      return;
    }
    case DEC_TYPE: {
      semty_t *ty = trans_ty(dec->type.ty);
      symtab_insert(tenv, dec->type.name, ty);
      return;
    }
  }
}

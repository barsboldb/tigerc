#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semant.h"

#define IS_ERROR(ty) ((ty) == error_type)

symtab_t *venv;
symtab_t *tenv;

static semty_t error_type_storage = { .kind = SEMTY_ERROR };
semty_t *error_type = &error_type_storage;
static int error_count = 0;
int semant_error_count(void) { return error_count; }

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
    case TY_NAME: {
      semty_t *res = symtab_lookup(tenv, ty->alias);
      if (!res) {
        fprintf(stderr, "error: unknown type\n");
        error_count++;
        return error_type;
      }
      return res;
    }
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
        error_count++;
        return error_type;
      }
      if (entry->kind != ENV_VAR) {
        fprintf(stderr, "error: cannot use function as a variable\n");
        error_count++;
        return error_type;
      }
      e->ty = entry->var;
      return entry->var;
    }
    case EXPR_FIELD: {
      semty_t *record_ty = trans_var(e->field_.record);
      if (IS_ERROR(record_ty))
        return error_type;

      if (record_ty->kind != SEMTY_RECORD) {
        fprintf(stderr, "error: field access is only allowed with record type\n");
        error_count++;
        return error_type;
      }
      field_ty_t *field_ty = record_ty->record;

      while (field_ty) {
        if (strcmp(field_ty->name, e->field_.field) == 0) break;
        field_ty = field_ty->next;
      }

      if (!field_ty) {
        fprintf(stderr, "error: unknown field '%s'\n", e->field_.field);
        error_count++;
        return error_type;
      }
      e->ty = field_ty->type;
      return field_ty->type;
    }
    case EXPR_INDEX: {
      semty_t *array_ty = trans_var(e->index_.array);

      if (IS_ERROR(array_ty))
        return error_type;

      if (array_ty->kind != SEMTY_ARRAY) {
        fprintf(stderr, "error: cannot access index of non-array variable\n");
        error_count++;
        return error_type;
      }
      semty_t *index_ty = trans_expr(e->index_.index);

      if (IS_ERROR(index_ty))
        return error_type;

      if (index_ty->kind != SEMTY_INT) {
        fprintf(stderr, "error: cannot access non-integer array index\n");
        error_count++;
        return error_type;
      }
      e->ty = array_ty->array;
      return array_ty->array;
    }
    default: return error_type;
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
      if (dec->func.type_name) {
        entry->func.ret = symtab_lookup(tenv, dec->func.type_name);
      } else {
        semty_t *void_ty = malloc(sizeof(semty_t));
        void_ty->kind = SEMTY_VOID;
        entry->func.ret = void_ty;
      }
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

      if (IS_ERROR(s))
        return error_type;

      e->ty = s;
      return s;
    case EXPR_ASSIGN: {
      semty_t *rhs = trans_var(e->assign.lhs);
      semty_t *lhs = trans_expr(e->assign.rhs);
      s->kind = SEMTY_VOID;
      if (IS_ERROR(lhs) || IS_ERROR(rhs)) return error_type;
      s->kind = SEMTY_VOID;
      int nil_to_record = rhs->kind == SEMTY_RECORD && lhs->kind == SEMTY_NIL;
      if (!nil_to_record && lhs->kind != rhs->kind) {
        fprintf(stderr, "error: type mismatch in assignment\n");
        error_count++;
      }
      e->ty = s;
      return s;
    }
    case EXPR_IF: {
      semty_t *cond = trans_expr(e->if_.cond);

      if (IS_ERROR(cond))
        return error_type;

      if (cond->kind != SEMTY_INT) {
        fprintf(stderr, "error: if expr condition must be int\n");
        error_count++;
        return error_type;
      }
      semty_t *then = trans_expr(e->if_.then);

      if (IS_ERROR(then))
        return error_type;

      if (!e->if_.else_) {
        return then;
      }
      semty_t *else_ = trans_expr(e->if_.else_);
      if (IS_ERROR(else_) && IS_ERROR(then)) {
        fprintf(stderr, "error: cannot infer return type, add a return type annotation");
        error_count++;
        return error_type;
      }
      if (!else_) {
        return then;
      }
      if (then->kind != else_->kind) {
        fprintf(stderr, "error: if expr type is ambigious\n");
        error_count++;
        return error_type;
      }
      e->ty = then;
      return then;
    }
    case EXPR_WHILE: {
      semty_t *cond = trans_expr(e->while_.cond);

      if (IS_ERROR(cond))
        return error_type;

      if (cond->kind != SEMTY_INT) {
        fprintf(stderr, "error: while expr condition must be int\n");
        error_count++;
        return error_type;
      }
      semty_t *body = trans_expr(e->while_.body);
      if (IS_ERROR(body)) return error_type;
      s->kind = SEMTY_VOID;
      e->ty = s;
      return s;
    }
    case EXPR_FOR: {
      semty_t *init = trans_expr(e->for_.init);
      semty_t *to   = trans_expr(e->for_.to);

      if (IS_ERROR(init) || IS_ERROR(to))
        return error_type;

      if (init->kind != to->kind) {
        fprintf(stderr, "error: for expr type mismatch\n");
        error_count++;
        return error_type;
      }
      if (init->kind != SEMTY_INT) {
        fprintf(stderr, "error: for expr iterator type should be int\n");
        error_count++;
				return error_type;
      }
      s->kind = SEMTY_VOID;
      e->ty = s;
      return s;
    }
    case EXPR_CALL: {
      env_entry_t *f = symtab_lookup(venv, e->call.id);
      if (!f) {
        fprintf(stderr, "error: undefined function '%s'\n", e->call.id);
        error_count++;
				return error_type;
      }
      if (f->kind == ENV_VAR) {
        fprintf(stderr, "error: cannot invoke variable call\n");
        error_count++;
				return error_type;
      }

      param_ty_t *p = f->func.params;
      expr_list_t *a = e->call.arg_list;
      int had_error = 0;
      while (p) {
        semty_t *arg_ty  = trans_expr(a->expr);

        if (IS_ERROR(arg_ty)) {
          had_error = 1;
          p = p->next;
          a = a->next;
          continue;
        }

        semty_t *param_ty = actual_ty(tenv, p->type);
        if (param_ty->kind != arg_ty->kind) {
          if (!(param_ty->kind == SEMTY_RECORD && arg_ty->kind == SEMTY_NIL)) {
            fprintf(stderr, "error: param type mismatch\n");
            error_count++;
            had_error = 1;
          }
        }
        p = p->next;
        a = a->next;
      }
      if (had_error) {
        return error_type;
      }
      e->ty = f->func.ret;
      e->call.callee_depth = f->func.depth;
      return f->func.ret;
    }
    case EXPR_SEQ: {
      expr_list_t *seq = e->seq;
      if (!seq) {
        s->kind = SEMTY_VOID;
        e->ty = s;
        return s;
      }
      int had_error = 0;
      while (seq->next) {
        if (IS_ERROR(trans_expr(seq->expr))) had_error = 1;
        seq = seq->next;
      }
      semty_t *ty = trans_expr(seq->expr);

      if (had_error || IS_ERROR(ty))
        return error_type;

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

      int had_error = 0;
      while (l) {
        result = trans_expr(l->expr);
        if (IS_ERROR(result)) {
          had_error = 1;
        }

        l = l->next;
      }
      symtab_exit_scope(venv);
      symtab_exit_scope(tenv);

      if (had_error) {
        return error_type;
      }

      e->ty = result;
      return result;
    }
    case EXPR_BINOP: {
      semty_t *l = trans_expr(e->binop.left);
      semty_t *r = trans_expr(e->binop.right);

      if (IS_ERROR(l) || IS_ERROR(r))
        return error_type;

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
            return error_type;
          }
          if (l->kind != SEMTY_INT) {
            fprintf(stderr, "error: operands should be ints\n");
            error_count++;
            return error_type;
          }
          if (r->kind != SEMTY_INT) {
            fprintf(stderr, "error: operands should be ints\n");
            error_count++;
            return error_type;
          }
          res->kind = SEMTY_INT;
          e->ty = res;
          return res;
        case OP_EQ:
        case OP_NEQ:
          l = actual_ty(tenv, l);
          r = actual_ty(tenv, r);
          if (l->kind != r->kind) {
            int nil_rec = (l->kind == SEMTY_RECORD && r->kind == SEMTY_NIL) ||
                          (l->kind == SEMTY_NIL && r->kind == SEMTY_RECORD);
            if (!nil_rec) {
              fprintf(stderr, "error: operands should be same type\n");
              error_count++;
              return error_type;
            }
          }
          res->kind = SEMTY_INT;
          e->ty = res;
          return res;
      }
    }
    case EXPR_RECORD: {
      semty_t *t = symtab_lookup(tenv, e->record.type_name);

      if (!t) {
        fprintf(stderr, "error: undeclared type '%s'\n", e->record.type_name);
        error_count++;
        return error_type;
      }

      if (t->kind != SEMTY_RECORD) {
        fprintf(stderr, "error: record type expected\n");
        error_count++;
        return error_type;
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

        if (IS_ERROR(s)) {
          field = field->next;
          continue;
        }

        if (s->kind == SEMTY_NIL) {
          field = field->next;
          continue;
        }
        
        s = actual_ty(tenv, s);
        if (!field_ty) {
          field = field->next;
          continue;
        }
        semty_t *f = actual_ty(tenv, field_ty->type);
        if (s->kind != f->kind) {
          fprintf(stderr, "error: record field type mismatch\n");
          error_count++;
        }
        field = field->next;
      }
      e->ty = t;
      return t;
    }
    case EXPR_ARRAY: {
      semty_t *semty = symtab_lookup(tenv, e->array.type_name);

      if (!semty) {
        error_count++;
        return error_type;
      }

      semty_t *size  = trans_expr(e->array.size);
      semty_t *init  = trans_expr(e->array.init);

      if (IS_ERROR(size) || IS_ERROR(init))
        return error_type;

      if (semty->kind != SEMTY_ARRAY) {
        fprintf(stderr, "error: array type expected\n");
        error_count++;
        return error_type;
      }
      if (size->kind != SEMTY_INT) {
        fprintf(stderr, "error: array size must be int\n");
        error_count++;
      }
      if (init->kind != semty->array->kind) {
        fprintf(stderr, "error: array init type mismatch\n");
        error_count++;
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

      if (body_ty && !IS_ERROR(body_ty) && body_ty->kind != entry->func.ret->kind) {
        fprintf(stderr, "error: function '%s' body type does not match declared return type\n", dec->func.id);
        error_count++;
      }
      current_depth--;
      return;
    }
    case DEC_VAR: {
      env_entry_t *s = malloc(sizeof(env_entry_t));
      s->kind = ENV_VAR;

      semty_t *init_ty = trans_expr(dec->var.init);

      if (IS_ERROR(init_ty)) {
        s->var = error_type;
        symtab_insert(venv, dec->var.id, s);
        return;
      }

      if (dec->var.type_name) {
        semty_t *declared = symtab_lookup(tenv, dec->var.type_name);

        int nil_to_record = declared->kind == SEMTY_RECORD && init_ty->kind == SEMTY_NIL;
        if (!nil_to_record && declared->kind != init_ty->kind) {
          fprintf(stderr, "error: type mismatch in var declaration '%s'\n", dec->var.id);
          error_count++;
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

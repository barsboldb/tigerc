#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semant.h"

semty_t *trans_ty(symtab_t *tenv, ty_t *ty) {
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

semty_t *trans_var(symtab_t *venv, symtab_t *tenv, expr_t *e) {
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
      return entry->var;
    }
    case EXPR_FIELD: {
      semty_t *record_ty = trans_var(venv, tenv, e->field_.record);
      if (record_ty->kind != SEMTY_RECORD) {
        fprintf(stderr, "error: field access is only allowed with record type\n");
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

      return field_ty->type;
    }
    case EXPR_INDEX: {
      semty_t *array_ty = trans_var(venv, tenv, e->index_.array);
      if (array_ty->kind != SEMTY_ARRAY) {
        fprintf(stderr, "error: cannot access index of non-array variable\n");
        return NULL;
      }
      semty_t *index_ty = trans_expr(venv, tenv, e->index_.index);
      if (!array_ty) {
      }
      if (index_ty->kind != SEMTY_INT) {
        fprintf(stderr, "error: cannot access non-integer array index\n");
        return NULL;
      }
      return array_ty->array;
    }
    default: return NULL;
  }
}

semty_t *trans_expr(symtab_t *venv, symtab_t *tenv, expr_t *e) {
  semty_t *s = malloc(sizeof(semty_t));
  switch (e->kind) {
    case EXPR_INT:
      s->kind = SEMTY_INT;
      return s;
    case EXPR_STRING:
      s->kind = SEMTY_STRING;
      return s;
    case EXPR_NIL:
      s->kind = SEMTY_NIL;
      return s;
    case EXPR_ID:
    case EXPR_FIELD:
    case EXPR_INDEX:
      s = trans_var(venv, tenv, e);
      return s;
    case EXPR_ASSIGN: {
      env_entry_t *rhs = symtab_lookup(venv, e->assign.var);
      semty_t *lhs = trans_expr(venv, tenv, e->assign.rhs);
      if (rhs->kind != ENV_VAR) {
        fprintf(stderr, "error: cannot assign to function expr\n");
        return NULL;
      }
      if (rhs->var->kind != lhs->kind) {
        fprintf(stderr, "error: type mismatch in assign '%s'\n", e->assign.var);
        return NULL;
      }
      s->kind = SEMTY_VOID;
      return s;
    }
    case EXPR_IF: {
      semty_t *cond = trans_expr(venv, tenv, e->if_.cond);
      if (cond->kind != SEMTY_INT) {
        fprintf(stderr, "error: if expr condition must be int\n");
        return NULL;
      }
      semty_t *then = trans_expr(venv, tenv, e->if_.then);
      if (!e->if_.else_) {
        return then;
      }
      semty_t *else_ = trans_expr(venv, tenv, e->if_.else_);
      if (then->kind != else_->kind) {
        fprintf(stderr, "error: if expr type is ambigious\n");
        return NULL;
      }
      return then;
    }
    case EXPR_WHILE: {
      semty_t *cond = trans_expr(venv, tenv, e->while_.cond);
      if (cond->kind != SEMTY_INT) {
        fprintf(stderr, "error: while expr condition must be int\n");
        return NULL;
      }
      trans_expr(venv, tenv, e->while_.body);
      s->kind = SEMTY_VOID;
      return s;
    }
    case EXPR_FOR: {
      semty_t *init = trans_expr(venv, tenv, e->for_.init);
      semty_t *to   = trans_expr(venv, tenv, e->for_.to);
      if (init->kind != to->kind) {
        fprintf(stderr, "error: for expr type mismatch\n");
        return NULL;
      }
      if (init->kind != SEMTY_INT) {
        fprintf(stderr, "error: for expr iterator type should be int\n");
        return NULL;
      }
      s->kind = SEMTY_VOID;
      return s;
    }
    case EXPR_CALL: {
      env_entry_t *f = symtab_lookup(venv, e->call.id);
      if (f->kind == ENV_VAR) {
        fprintf(stderr, "error: cannot invoke variable call\n");
        return NULL;
      }

      param_ty_t *p = f->func.params;
      expr_list_t *a = e->call.arg_list;
      while (p) {
        if (p->type->kind != trans_expr(venv, tenv, a->expr)->kind) {
          fprintf(stderr, "error: param type mismatch\n");
          return NULL;
        }
        p = p->next;
        a = a->next;
      }
      return f->func.ret;
    }
    case EXPR_SEQ: {
      expr_list_t *seq = e->seq;
      while (seq->next) {
        trans_expr(venv, tenv, seq->expr);
        seq = seq->next;
      }
      return trans_expr(venv, tenv, seq->expr);
    }
    case EXPR_LET: {
      symtab_enter_scope(venv);
      symtab_enter_scope(tenv);
      dec_list_t *d = e->let.dec_list;
      while (d) {
        trans_dec(venv, tenv, d->dec);
        d = d->next;
      }
      semty_t *result = NULL;
      expr_list_t *l = e->let.body;
      while (l) {
        result = trans_expr(venv, tenv, l->expr);
        l = l->next;
      }
      symtab_exit_scope(venv);
      symtab_exit_scope(tenv);
      return result;
    }
    case EXPR_BINOP: {
      semty_t *l = trans_expr(venv, tenv, e->binop.left);
      semty_t *r = trans_expr(venv, tenv, e->binop.right);
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
          if (l->kind != SEMTY_INT) {
            fprintf(stderr, "error: operands should be ints\n");
            return NULL;
          }
          if (r->kind != SEMTY_INT) {
            fprintf(stderr, "error: operands should be ints\n");
            return NULL;
          }
          res->kind = SEMTY_INT;
          return res;
        case OP_EQ:
        case OP_NEQ:
          if (l->kind != r->kind) {
            fprintf(stderr, "error: operands should be same type\n");
            return NULL;
          }
          res->kind = SEMTY_INT;
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

        semty_t *s = trans_expr(venv, tenv, field->val);
        if (s->kind != field_ty->type->kind) {
          fprintf(stderr, "error: record field type mismatch\n");
        }
        field = field->next;
      }
      return t;
    }
    case EXPR_ARRAY: {
      semty_t *semty = symtab_lookup(tenv, e->array.type_name);
      semty_t *size  = trans_expr(venv, tenv, e->array.size);
      semty_t *init  = trans_expr(venv, tenv, e->array.init);

      if (semty->kind != SEMTY_ARRAY) {
        fprintf(stderr, "error: array type expected\n");
      }
      if (size->kind != SEMTY_INT) {
        fprintf(stderr, "error: array size must be int\n");
      }
      if (init->kind != semty->array->kind) {
        fprintf(stderr, "error: array init type mismatch\n");
      }
      
      return semty;
    }
  }
}

void trans_dec(symtab_t *venv, symtab_t *tenv, dec_t *dec) {
  switch (dec->kind) {
    case DEC_FUNC: {
      env_entry_t *s = malloc(sizeof(env_entry_t));
      s->kind = ENV_FUNC;
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
      s->func.params = p_ty;
      s->func.ret = dec->func.type_name
        ? symtab_lookup(tenv, dec->func.type_name)
        : NULL;
      symtab_insert(venv, dec->func.id, s);
      return;
    }
    case DEC_VAR: {
      env_entry_t *s = malloc(sizeof(env_entry_t));
      s->kind = ENV_VAR;

      semty_t *init_ty = trans_expr(venv, tenv, dec->var.init);
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
      semty_t *ty = trans_ty(tenv, dec->type.ty);
      symtab_insert(tenv, dec->type.name, ty);
      return;
    }
  }
}

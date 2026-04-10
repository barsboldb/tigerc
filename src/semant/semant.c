#include <stdio.h>
#include <stdlib.h>
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
  (void)venv; (void)tenv; (void)e;
  return NULL;
}

semty_t *trans_expr(symtab_t *venv, symtab_t *tenv, expr_t *e) {
  (void)venv; (void)tenv; (void)e;
  return NULL;
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
          fprintf(stderr, "error: type mismatch invar declaration '%s'\n", dec->var.id);
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

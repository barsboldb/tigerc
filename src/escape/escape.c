#include <stdlib.h>
#include "escape.h"
#include "symtab.h"

typedef struct {
  int   depth;
  int  *escape;
} esc_entry_t;

static void escape_dec(symtab_t *escenv, dec_t *d, int depth);

static void escape_expr(symtab_t *escenv, expr_t *e, int depth) {
  switch (e->kind) {
    case EXPR_ID: {
      esc_entry_t *entry = symtab_lookup(escenv, e->id);
      if (entry && entry->depth < depth) {
        *entry->escape = 1;
      }
      break;
    }
    case EXPR_INDEX: 
      escape_expr(escenv, e->index_.array, depth);
      escape_expr(escenv, e->index_.index, depth);
      break;
    case EXPR_FIELD:
      escape_expr(escenv, e->field_.record, depth);
      break;
    case EXPR_BINOP:
      escape_expr(escenv, e->binop.left, depth);
      escape_expr(escenv, e->binop.right, depth);
      break;
    case EXPR_ASSIGN: {
      esc_entry_t *entry = symtab_lookup(escenv, e->assign.var);
      if (entry && entry->depth < depth) *entry->escape = 1;
      escape_expr(escenv, e->assign.rhs, depth);
      break;
    }
    case EXPR_IF:
      escape_expr(escenv, e->if_.cond, depth);
      escape_expr(escenv, e->if_.then, depth);
      if (e->if_.else_)
        escape_expr(escenv, e->if_.else_, depth);
      break;
    case EXPR_WHILE:
      escape_expr(escenv, e->while_.cond, depth);
      escape_expr(escenv, e->while_.body, depth);
      break;
    case EXPR_FOR: {
      esc_entry_t *entry = malloc(sizeof(esc_entry_t));
      entry->depth = depth;
      entry->escape = &e->for_.escape;
      symtab_insert(escenv, e->for_.var, entry);
      escape_expr(escenv, e->for_.init, depth);
      escape_expr(escenv, e->for_.to, depth);
      escape_expr(escenv, e->for_.body, depth);
      break;
    }
    case EXPR_LET: {
      symtab_enter_scope(escenv);
      dec_list_t *dl = e->let.dec_list;
      while (dl) {
        escape_dec(escenv, dl->dec, depth);
        dl = dl->next;
      }

      expr_list_t *el = e->let.body;
      while (el) {
        escape_expr(escenv, el->expr, depth);
        el = el->next;
      }
      symtab_exit_scope(escenv);
      break;
    }
    case EXPR_CALL: {
      expr_list_t *el = e->call.arg_list;
      while (el) {
        escape_expr(escenv, el->expr, depth);
        el = el->next;
      }
      break;
    }
    case EXPR_SEQ: {
      expr_list_t *el = e->seq;
      while (el) {
        escape_expr(escenv, el->expr, depth);
        el = el->next;
      }
      break;
    }
    case EXPR_RECORD: {
      field_list_t *fl = e->record.fields;
      while (fl) {
        escape_expr(escenv, fl->val, depth);
        fl = fl->next;
      }
      break;
    }
    case EXPR_ARRAY:
      escape_expr(escenv, e->array.init, depth);
      escape_expr(escenv, e->array.size, depth);
      break;
    default: return;
  }
}

static void escape_dec(symtab_t *escenv, dec_t *d, int depth) {
  switch (d->kind) {
    case DEC_VAR: {
      esc_entry_t *e = malloc(sizeof(esc_entry_t));
      e->depth = depth;
      e->escape = &d->var.escape;
      symtab_insert(escenv, d->var.id, e);
      escape_expr(escenv, d->var.init, depth);
      break;
    }
    case DEC_FUNC:
      symtab_enter_scope(escenv);
      param_list_t *p = d->func.args;
      while (p) {
        esc_entry_t *pe = malloc(sizeof(esc_entry_t));
        pe->depth = depth + 1;
        pe->escape = &p->param->escape;
        symtab_insert(escenv, p->param->name, pe);
        p = p->next;
      }
      if (d->func.body) {
        escape_expr(escenv, d->func.body, depth + 1);
      }
      symtab_exit_scope(escenv);
      break;
    default:
      return;
  }
}

void find_escape(expr_t *prog) {
  symtab_t *escenv = symtab_new(64);
  symtab_enter_scope(escenv);
  escape_expr(escenv, prog, 0);
}

#ifndef SEMANT_H
#define SEMANT_H

#include "types.h"
#include "symtab.h"
#include "ast.h"

typedef enum {
  ENV_VAR,
  ENV_FUNC,
} env_kind_t;

typedef struct param_ty_t {
  semty_t           *type;
  struct param_ty_t *next;
} param_ty_t;

typedef struct {
  env_kind_t kind;
  union {
    semty_t *var;
    struct {
      param_ty_t *params;
      semty_t    *ret;
    } func;
  };
} env_entry_t;

symtab_t *semant_base_venv(symtab_t *tenv);
symtab_t *semant_base_tenv();

semty_t *trans_expr(symtab_t *venv, symtab_t *tenv, expr_t *e);
semty_t *trans_var(symtab_t *venv, symtab_t *tenv, expr_t *e);
semty_t *trans_ty(symtab_t *tenv, ty_t *ty);
void     trans_dec_header(symtab_t *venv, symtab_t *tenv, dec_t *d);
void     trans_dec(symtab_t *venv, symtab_t *tenv, dec_t *d);

#endif

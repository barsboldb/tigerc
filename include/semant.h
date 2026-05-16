#ifndef SEMANT_H
#define SEMANT_H

#include "types.h"
#include "symtab.h"
#include "ast.h"

extern symtab_t *venv;
extern symtab_t *tenv;

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
      int         depth;
    } func;
  };
} env_entry_t;

symtab_t *semant_base_venv(symtab_t *tenv);
symtab_t *semant_base_tenv();

semty_t *trans_expr(expr_t *e);
semty_t *trans_var(expr_t *e);
semty_t *trans_ty(ty_t *ty);
void     trans_dec_header(dec_t *d);
void     trans_dec(dec_t *d);

#endif

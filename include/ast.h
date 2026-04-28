#ifndef AST_H
#define AST_H

typedef enum {
  EXPR_INT,
  EXPR_STRING,
  EXPR_NIL,

  EXPR_ID,
  EXPR_INDEX,
  EXPR_FIELD,
  EXPR_BINOP,
  EXPR_ASSIGN,
  EXPR_IF,
  EXPR_WHILE,
  EXPR_FOR,
  EXPR_LET,
  EXPR_CALL,
  EXPR_SEQ,
  EXPR_RECORD,
  EXPR_ARRAY,
} expr_kind_t;

typedef enum {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_EQ,
  OP_NEQ,
  OP_LT,
  OP_LE,
  OP_GT,
  OP_GE,
  OP_AND,
  OP_OR,
} binop_t;

typedef enum {
  TY_NAME,
  TY_RECORD,
  TY_ARRAY,
} ty_kind_t;

typedef struct expr_list_t {
  struct expr_t      *expr;
  struct expr_list_t *next;
} expr_list_t;

typedef struct param_t {
  char *name;
  char *type_name;
  int   escape;
} param_t;

typedef struct param_list_t {
  param_t             *param;
  struct param_list_t *next;
} param_list_t;

typedef enum {
  DEC_VAR,
  DEC_FUNC,
  DEC_TYPE,
} dec_kind_t;

typedef struct ty_t {
  ty_kind_t kind;
  union {
    char         *alias;
    param_list_t *fields;
    char         *array_of;
  };
} ty_t;

typedef struct dec_t {
  dec_kind_t kind;
  union {
    struct {
      char          *id;
      char          *type_name;
      struct expr_t *init;
      int            escape;
    } var;

    struct {
      char          *id;
      char          *type_name;
      param_list_t  *args; 
      struct expr_t *body;
    } func;

    struct {
      char *name;
      ty_t *ty;
    } type;
  };
} dec_t;

typedef struct dec_list_t {
  struct dec_t *dec;
  struct dec_list_t *next;
} dec_list_t;

typedef struct field_list_t {
  char                *name;
  struct expr_t       *val;
  struct field_list_t *next;
} field_list_t;

typedef struct expr_t {
  expr_kind_t kind;
  union {
    int   int_val;
    char *str_val;
    char *id;
    struct {
      struct expr_t *array;
      struct expr_t *index;
    } index_;

    struct {
      struct expr_t *record;
      char          *field;
    } field_;

    struct {
      struct expr_t *left;
      struct expr_t *right;
      binop_t op;
    } binop;

    struct {
      struct expr_t *cond;
      struct expr_t *then;
      struct expr_t *else_;
    } if_;

    struct {
      struct expr_t *cond;
      struct expr_t *body;
    } while_;

    struct {
      char          *var;
      struct expr_t *init;
      struct expr_t *to;
      struct expr_t *body;
      int            escape;
    } for_;

    struct {
      char          *var;
      struct expr_t *rhs;
    } assign;

    struct {
      char        *id;
      expr_list_t *arg_list;
    } call;

    expr_list_t *seq;

    struct {
      dec_list_t         *dec_list;
      struct expr_list_t *body;
    } let;

    struct {
      char         *type_name;
      field_list_t *fields;
    } record;

    struct {
      char *type_name;
      struct expr_t *size;
      struct expr_t *init;
    } array;
  };
  int line, col;
} expr_t;

#endif

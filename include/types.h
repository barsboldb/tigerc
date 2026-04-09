#ifndef TYPES_H
#define TYPES_H

typedef enum {
  SEMTY_INT,
  SEMTY_STRING,
  SEMTY_NIL,
  SEMTY_VOID,
  SEMTY_RECORD,
  SEMTY_ARRAY,
  SEMTY_NAME,
} semty_kind_t;

typedef struct field_ty_t {
  char              *name;
  struct semty_t    *type;
  struct field_ty_t *next;
} field_ty_t;

typedef struct semty_t {
  semty_kind_t kind;
  union {
    struct field_ty_t *record;
    struct semty_t    *array;
    char              *name;
  };
} semty_t;

#endif

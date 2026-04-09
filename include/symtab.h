#ifndef SYMTAB_H
#define SYMTAB_H

typedef struct bucket_t {
  char            *key;
  void            *val;
  struct bucket_t *next;
} bucket_t;

typedef struct undo_t {
  bucket_t      *bucket;
  void          *oldval;
  int            is_mark;
  struct undo_t *next;
} undo_t;

typedef struct {
  bucket_t **buckets;
  undo_t    *undo_stack;
  int        size;
} symtab_t;

symtab_t *symtab_new(int size);
void      symtab_enter_scope(symtab_t *tab);
void      symtab_exit_scope(symtab_t *tab);
void      symtab_insert(symtab_t *tab, char *key, void *val);
void     *symtab_lookup(symtab_t *tab, char *key);

#endif

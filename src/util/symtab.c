#include <stdlib.h>
#include <string.h>
#include "symtab.h"

static int hash(symtab_t *tab, char *s) {
  unsigned h = 0;
  while (*s) h = h * 65599 + (unsigned char)*s++;
  return h % tab->size;
}

static bucket_t *tab_lookup(bucket_t *b, char *key) {
  while (b && strcmp(b->key, key) != 0) {
    b = b->next;
  }
  return b;
}

static undo_t *insert_undo(undo_t *undo, bucket_t *bucket, void *oldval) {
  undo_t *node  = malloc(sizeof(undo_t));
  node->bucket  = bucket;
  node->oldval  = oldval;
  node->is_mark = 0;
  node->next    = undo;
  return node;
}

symtab_t *symtab_new(int size) {
  symtab_t *tab = malloc(sizeof(symtab_t));

  tab->buckets = calloc(size, sizeof(bucket_t *));
  tab->size = size;
  tab->undo_stack = NULL;

  return tab;
}

void symtab_insert(symtab_t *tab, char *key, void *val) {
  int h = hash(tab, key);
  bucket_t *b = tab_lookup(tab->buckets[h], key);

  if (b) {
    tab->undo_stack = insert_undo(tab->undo_stack, b, b->val);
    b->val = val;
    return;
  }

  bucket_t *new = malloc(sizeof(bucket_t));
  new->val = val;
  new->key = key;
  new->next = tab->buckets[h];
  tab->buckets[h] = new;
  tab->undo_stack = insert_undo(tab->undo_stack, new, NULL);
}

void *symtab_lookup(symtab_t *tab, char *key) {
  int h = hash(tab, key);
  bucket_t *b = tab_lookup(tab->buckets[h], key);
  if (b) return b->val;
  else   return NULL;
}

void symtab_enter_scope(symtab_t *tab) {
  undo_t *node  = malloc(sizeof(undo_t));
  node->bucket  = NULL;
  node->oldval  = NULL;
  node->is_mark = 1;
  node->next    = tab->undo_stack;
  tab->undo_stack = node;
}

void symtab_exit_scope(symtab_t *tab) {
  while (tab->undo_stack && tab->undo_stack->is_mark == 0) {
    undo_t *u = tab->undo_stack;
    u->bucket->val = u->oldval;
    tab->undo_stack = u->next;
  }

  if (tab->undo_stack)
    tab->undo_stack = tab->undo_stack->next;
}

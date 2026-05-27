#ifndef BITSET_H
#define BITSET_H
#include <stdint.h>

typedef struct {
  uint64_t *words;
  int       nwords;
} bitset_t;

bitset_t *bitset_new(int capacity);
void      bitset_add(bitset_t *s, int i);
void      bitset_remove(bitset_t *s, int i);
int       bitset_has(bitset_t *s, int i);
void      bitset_union(bitset_t *dst, bitset_t *src);
void      bitset_diff(bitset_t *dst, bitset_t *src);
int       bitset_equal(bitset_t *a, bitset_t *b);
bitset_t *bitset_copy(bitset_t *s);

#endif

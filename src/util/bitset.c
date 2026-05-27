#include <stdlib.h>
#include "bitset.h"

#define min(x, y) ((x) > (y) ? (y) : (x))

bitset_t *bitset_new(int capacity) {
  int nwords = (capacity + 63) / 64;
  bitset_t *bs = malloc(sizeof(bitset_t));
  bs->words = calloc(nwords, sizeof(uint64_t));
  bs->nwords = nwords;
  return bs;
}

void bitset_add(bitset_t *s, int i) {
  s->words[i/64] |= (1ULL << (i%64));
}

void bitset_remove(bitset_t *s, int i) {
  s->words[i/64] &= ~(1ULL << (i%64));
}

int bitset_has(bitset_t *s, int i) {
  return (s->words[i/64] >> (i%64)) & 1;
}

void bitset_union(bitset_t *dst, bitset_t *src) {
  for (int i = 0; i < min(dst->nwords, src->nwords); i++) {
    dst->words[i] |= src->words[i];
  }
}

void bitset_diff(bitset_t *dst, bitset_t *src) {
  for (int i = 0; i < min(dst->nwords, src->nwords); i++)
    dst->words[i] &= ~src->words[i];
}

int bitset_equal(bitset_t *a, bitset_t *b) {
  int n = min(a->nwords, b->nwords);
  for (int i = 0; i < n; i++)
    if (a->words[i] != b->words[i]) return 0;
  return 1;
}

bitset_t *bitset_copy(bitset_t *s) {
  bitset_t *c = malloc(sizeof(bitset_t));
  c->nwords = s->nwords;
  c->words = malloc(s->nwords * sizeof(uint64_t));
  for (int i = 0; i < s->nwords; i++)
    c->words[i] = s->words[i];
  return c;
}

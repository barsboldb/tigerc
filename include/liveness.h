#ifndef LIVENESS_H
#define LIVENESS_H
#include "codegen.h"
#include "bitset.h"

typedef struct cfg_node_t {
  instr_t            *instr;
  struct cfg_node_t **succs;
  int                 num_succs;
  struct cfg_node_t  *next;
  bitset_t           *in;
  bitset_t           *out;
} cfg_node_t;

typedef struct {
  cfg_node_t *nodes;
  cfg_node_t *last;
} cfg_t;

cfg_t *cfg_build(instr_t *instrs);
void liveness(cfg_t *cfg);

#endif

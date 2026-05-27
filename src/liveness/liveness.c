#include <stdlib.h>
#include "liveness.h"
#include "symtab.h"
#include "temp.h"

cfg_t *cfg_build(instr_t *instrs) {
  cfg_t *cfg = malloc(sizeof(cfg_t));
  cfg->nodes = cfg->last = NULL;

  for (instr_t *i = instrs; i; i = i->next) {
    cfg_node_t *n = malloc(sizeof(cfg_node_t));
    n->instr     = i;
    n->succs     = NULL;
    n->num_succs = 0;
    n->next      = NULL;
    n->in        = bitset_new(temp_count());
    n->out       = bitset_new(temp_count());

    if (cfg->last) cfg->last = cfg->last->next = n;
    else           cfg->nodes = cfg->last = n;
  }

  symtab_t *tab = symtab_new(64);
  symtab_enter_scope(tab);
  for (cfg_node_t *n = cfg->nodes; n; n = n->next) {
    if (n->instr->kind == INSTR_LABEL)
      symtab_insert(tab, label_name(n->instr->label.label), n);
  }

  for (cfg_node_t *i = cfg->nodes; i; i = i->next) {
    if (i->instr->kind == INSTR_OPER && i->instr->oper.jumps_num > 0) {
      i->num_succs = i->instr->oper.jumps_num;
      i->succs = malloc(i->instr->oper.jumps_num * sizeof(cfg_node_t *));
      for (int j = 0; j < i->num_succs; j++) {
        i->succs[j] = symtab_lookup(tab, label_name(i->instr->oper.jumps[j]));
      }
      continue;
    } else if (i->next) {
      i->num_succs = 1;
      i->succs = malloc(sizeof(cfg_node_t *));
      i->succs[0] = i->next;
    }
  }

  return cfg;
}

void liveness(cfg_t *cfg) {
  while (1) {
    int changed = 0;

    for (cfg_node_t *n = cfg->nodes; n; n = n->next) {
      bitset_t *use = bitset_new(temp_count());
      bitset_t *def = bitset_new(temp_count());

      if (n->instr->kind == INSTR_MOVE) {
        bitset_add(use, n->instr->move.src);
        bitset_add(def, n->instr->move.dst);
      } else if (n->instr->kind == INSTR_OPER) {
        for (int i = 0; i < n->instr->oper.src_num; i++)
          bitset_add(use, n->instr->oper.src[i]);
        for (int i = 0; i < n->instr->oper.dst_num; i++)
          bitset_add(def, n->instr->oper.dst[i]);
      }

      bitset_t *new_out = bitset_new(temp_count());
      for (int i = 0; i < n->num_succs; i++) {
        if (n->succs[i])
          bitset_union(new_out, n->succs[i]->in);
      }

      bitset_t *new_in = bitset_copy(new_out);
      bitset_diff(new_in, def);
      bitset_union(new_in, use);

      if (!bitset_equal(new_in, n->in) || !bitset_equal(new_out, n->out)) {
        changed = 1;
        n->in = new_in;
        n->out = new_out;
      } else {
      }
    }

    if (!changed) break;
  }
}

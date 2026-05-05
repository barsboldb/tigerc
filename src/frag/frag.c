#include "stdlib.h"
#include "frag.h"

static frag_t *frags = NULL;

void frag_insert(frag_t *f) {
  f->next = frags;
  frags = f;
}

frag_t *frag_list(void) {
  return frags;
}

frag_t *frag_str(label_t label, char *str) {
  frag_t *f = malloc(sizeof(frag_t));

  f->kind          = FRAG_STRING;
  f->string_.label = label;
  f->string_.str   = str;

  return f;
}

frag_t *frag_proc(frame_t *frame, tree_stmt_t *stmt) {
  frag_t *f = malloc(sizeof(frag_t));

  f->kind        = FRAG_PROC;
  f->proc_.frame = frame;
  f->proc_.body  = stmt;

  return f;
}

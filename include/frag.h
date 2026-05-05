#ifndef FRAG_H
#define FRAG_H

#include "temp.h"
#include "frame.h"
#include "tree.h"

typedef enum {
  FRAG_STRING,
  FRAG_PROC,
} frag_kind_t;

typedef struct frag_t {
  frag_kind_t kind;
  union {
    struct {
      label_t  label;
      char    *str;
    } string_;
    struct {
      frame_t *frame;
      tree_stmt_t *body;
    } proc_;
  };
  struct frag_t *next;
} frag_t;

void    frag_insert(frag_t *f);
frag_t *frag_list(void);

frag_t *frag_str(label_t label, char *str);
frag_t *frag_proc(frame_t *frame, tree_stmt_t *stmt);

#endif

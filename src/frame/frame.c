#include <stdlib.h>
#include "frame.h"

frame_t  *frame_new(char *name, int *escapes, int num_params) {
  frame_t *frame = malloc(sizeof(frame_t));
  frame->name        = name;
  frame->local_count = 0;
  frame->formals     = malloc(sizeof(access_t) * num_params);
  frame->num_formals = num_params;
  int reg_index = 0;

  for (int i = 0; i < num_params; i++) {
    if (escapes[i]) {
      frame->formals[i].kind = ACCESS_FRAME;
      frame->formals[i].offset = -WORD_SIZE * (frame->local_count + 1);
      frame->local_count++;
    } else {
      frame->formals[i].kind = ACCESS_REG;
      frame->formals[i].reg  = reg_index++;
    }
  }
  return frame;
}

access_t *frame_alloc_local(frame_t *f, int escapes) {
  access_t *a = malloc(sizeof(access_t));
  if (escapes) {
    f->local_count++;
    a->kind   = ACCESS_FRAME;
    a->offset = -WORD_SIZE * f->local_count;
  } else {
    a->kind = ACCESS_REG;
    a->reg  = REG_TBD;
  }
  return a;
}

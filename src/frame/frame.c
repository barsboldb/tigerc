#include <stdlib.h>
#include "frame.h"
#include "temp.h"

const int WORD_SIZE = 8;

frame_t  *frame_new(char *name, int *escapes, int num_params, int depth) {
  int i;
  frame_t *frame = malloc(sizeof(frame_t));
  frame->name  = name;
  frame->depth = depth;
  if (depth > 0) {
    frame->formals = malloc(sizeof(access_t) * (num_params + 1));
    frame->formals[0].kind   = ACCESS_FRAME;
    frame->formals[0].offset = -WORD_SIZE;
    frame->formals[0].depth  = depth;
    frame->local_count       = i = 1;
    frame->num_formals       = num_params + 1;
  } else {
    frame->formals     = malloc(sizeof(access_t) * num_params);
    frame->local_count = i = 0;
    frame->num_formals = num_params;
  }
  for (int ei = 0; ei < num_params; ei++, i++) {
    if (escapes[ei]) {
      frame->formals[i].kind = ACCESS_FRAME;
      frame->formals[i].offset = -WORD_SIZE * (frame->local_count + 1);
      frame->local_count++;
    } else {
      frame->formals[i].kind = ACCESS_REG;
      frame->formals[i].reg  = temp_new();
    }
    frame->formals[i].depth = depth;
  }
  return frame;
}

access_t *frame_alloc_local(frame_t *f, int escape) {
  access_t *a = malloc(sizeof(access_t));
  a->depth = f->depth;
  if (escape) {
    f->local_count++;
    a->kind   = ACCESS_FRAME;
    a->offset = -WORD_SIZE * f->local_count;
  } else {
    a->kind = ACCESS_REG;
    a->reg  = temp_new();
  }
  return a;
}

temp_t frame_arg_reg(int i) {
  static temp_t regs[6] = {-1, -1, -1, -1, -1, -1};
  if (regs[i] == -1) regs[i] = temp_new();
  return regs[i];
}

temp_t frame_fp(void) {
  static temp_t fp = -1;
  if (fp == -1) fp = temp_new();
  return fp;
}

temp_t frame_rv(void) {
  static temp_t rv = -1;
  if (rv == -1) rv = temp_new();
  return rv;
}

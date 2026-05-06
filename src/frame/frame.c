#include <stdlib.h>
#include "frame.h"
#include "temp.h"

const int WORD_SIZE = 8;

frame_t  *frame_new(char *name, int *escapes, int num_params) {
  frame_t *frame = malloc(sizeof(frame_t));
  frame->name        = name;
  frame->local_count = 0;
  frame->formals     = malloc(sizeof(access_t) * num_params);
  frame->num_formals = num_params;
  for (int i = 0; i < num_params; i++) {
    if (escapes[i]) {
      frame->formals[i].kind = ACCESS_FRAME;
      frame->formals[i].offset = -WORD_SIZE * (frame->local_count + 1);
      frame->local_count++;
    } else {
      frame->formals[i].kind = ACCESS_REG;
      frame->formals[i].reg  = temp_new();
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
    a->reg  = temp_new();
  }
  return a;
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

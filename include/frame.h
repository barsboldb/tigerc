#ifndef FRAME_H
#define FRAME_H

#define REG_TBD   -1

#include "temp.h"

extern const int WORD_SIZE;

typedef enum {
  ACCESS_FRAME,
  ACCESS_REG
} access_kind_t;

typedef struct access_t {
  access_kind_t kind;
  union {
    int offset;
    int reg;
  };
} access_t;

typedef struct frame_t {
  char     *name;
  access_t *formals;
  int       num_formals;
  int       local_count;
} frame_t;

frame_t  *frame_new(char *name, int *escapes, int num_params);
access_t *frame_alloc_local(frame_t *f, int escapes);
temp_t frame_fp(void);
temp_t frame_rv(void);

#endif

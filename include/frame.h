#ifndef FRAME_H
#define FRAME_H

#define WORD_SIZE  8
#define REG_TBD   -1

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

#endif
